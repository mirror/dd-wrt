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
 * @file lac_ln.c Large Number API Implementation
 *
 * @ingroup LacAsymLn
 *
 * @description
 *      This file contains the implementation of Large Number
 *      ModExp and ModEnv functions
 *
 ***************************************************************************/

/*
********************************************************************************
* Include public/global header files
********************************************************************************
*/

/* Include API files */
#include "cpa.h"
#include "cpa_cy_ln.h"
#include "cpa_cy_common.h"

/* Include Osal files */
#include "IxOsal.h"

/* Include QAT files */
#include "icp_qat_fw_mmp.h"
#include "icp_qat_fw_mmp_ids.h"

/* Include LAC files */
#include "lac_common.h"
#include "lac_log.h"
#include "lac_pke_qat_comms.h"
#include "lac_mem.h"
#include "lac_mem_pools.h"
#include "lac_hooks.h"
#include "lac_ln.h"
#include "lac_pke_utils.h"

#include "lac_module.h"
/*
********************************************************************************
* Include private header files
********************************************************************************
*/

/*
********************************************************************************
* Static Variables
********************************************************************************
*/

/* number of Large Number (LN) Service stats */
#define LAC_LN_NUM_STATS (sizeof(CpaCyLnStats) / sizeof(Cpa32U))

/* array of atomics for Large Number (LN) Service stats */
STATIC IxOsalAtomic lacLnStats[LAC_LN_NUM_STATS];

/* macro to initialize all Large Number (LN) Service stats
* (stored in internal array of atomics) */
#define LAC_LN_STATS_INIT()                                         \
do {                                                                \
    Cpa32U i;                                                       \
                                                                    \
    for (i = 0; i < LAC_LN_NUM_STATS; i++)                          \
    {                                                               \
        ixOsalAtomicSet(0, &lacLnStats[i]);                         \
    }                                                               \
} while (0)

/* macro to increment a Large Number (LN) Service stat
* (derives offset into array of atomics) */


#define LAC_LN_STAT_INC(statistic, instanceHandle)                  \
do {                                                                \
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);                      \
                                                                    \
    ixOsalAtomicInc(                                                \
        &lacLnStats[                                                \
            offsetof(CpaCyLnStats, statistic) / sizeof(Cpa32U)]);   \
} while (0)



/* macro to get Large Number (LN) Service stats (from internal array of
 * atomics)
 *  into user supplied structure CpaCyLnStats pointed by lnStats
 *  pointer */
#define LAC_LN_STATS_GET(lnStats)                                       \
do {                                                                    \
    Cpa32U i;                                                           \
                                                                        \
    for (i = 0; i < LAC_LN_NUM_STATS; i++)                              \
    {                                                                   \
        ((Cpa32U *)&(lnStats))[i] = ixOsalAtomicGet(&lacLnStats[i]);    \
    }                                                                   \
} while (0)




/* Maps between operation sizes and Large Number Modular Exponentiation
*  function ids */
static const Cpa32U LAC_MATHS_MODEXP_SIZE_ID_MAP[][LAC_PKE_NUM_COLUMNS] =
{
    {LAC_512_BITS, MATHS_MODEXP_L512},
    {LAC_1024_BITS, MATHS_MODEXP_L1024},
    {LAC_1536_BITS, MATHS_MODEXP_L1536},
    {LAC_2048_BITS, MATHS_MODEXP_L2048},
    {LAC_2560_BITS, MATHS_MODEXP_L2560},
    {LAC_3072_BITS, MATHS_MODEXP_L3072},
    {LAC_3584_BITS, MATHS_MODEXP_L3584},
    {LAC_4096_BITS, MATHS_MODEXP_L4096}
};

/* Maps between operation sizes and Large Number Modular Inversion Odd
*  function ids */
static const Cpa32U LAC_MATHS_MODINV_ODD_SIZE_ID_MAP[][LAC_PKE_NUM_COLUMNS] =
{
    {LAC_128_BITS, MATHS_MODINV_ODD_L128},
    {LAC_192_BITS, MATHS_MODINV_ODD_L192},
    {LAC_256_BITS, MATHS_MODINV_ODD_L256},
    {LAC_384_BITS, MATHS_MODINV_ODD_L384},
    {LAC_512_BITS, MATHS_MODINV_ODD_L512},
    {LAC_768_BITS, MATHS_MODINV_ODD_L768},
    {LAC_1024_BITS, MATHS_MODINV_ODD_L1024},
    {LAC_1536_BITS, MATHS_MODINV_ODD_L1536},
    {LAC_2048_BITS, MATHS_MODINV_ODD_L2048},
    {LAC_3072_BITS, MATHS_MODINV_ODD_L3072},
    {LAC_4096_BITS, MATHS_MODINV_ODD_L4096}    
};

/* Maps between operation sizes and Large Number Modular Inversion Even
*  function ids */
static const Cpa32U LAC_MATHS_MODINV_EVEN_SIZE_ID_MAP[][LAC_PKE_NUM_COLUMNS] =
{
    {LAC_128_BITS, MATHS_MODINV_EVEN_L128},
    {LAC_192_BITS, MATHS_MODINV_EVEN_L192},
    {LAC_256_BITS, MATHS_MODINV_EVEN_L256},
    {LAC_384_BITS, MATHS_MODINV_EVEN_L384},
    {LAC_512_BITS, MATHS_MODINV_EVEN_L512},
    {LAC_768_BITS, MATHS_MODINV_EVEN_L768},
    {LAC_1024_BITS, MATHS_MODINV_EVEN_L1024},
    {LAC_1536_BITS, MATHS_MODINV_EVEN_L1536},
    {LAC_2048_BITS, MATHS_MODINV_EVEN_L2048},
    {LAC_3072_BITS, MATHS_MODINV_EVEN_L3072},
    {LAC_4096_BITS, MATHS_MODINV_EVEN_L4096}
};


/*
********************************************************************************
* Define static function definitions
********************************************************************************
*/


/*
********************************************************************************
* Global Variables
********************************************************************************
*/

/*
********************************************************************************
* Define public/global function definitions
********************************************************************************
*/

/**
 *******************************************************************************
 * @ingroup LacAsymLn
 *      Ln Get Buffer Data Size in Bytes function
 * The function scans the input buffer to locate the most significant non-zero
 * byte of a number in big endian order.
 ******************************************************************************/
STATIC
Cpa32U
LacGetBufferDataSizeInBytes(const CpaFlatBuffer *pBuffer)
{
    Cpa32U dataSizeInBytes = 0;
    Cpa32U counter = 0;
    
    if (NULL != pBuffer)
    {
        Cpa32U maxDataSize = pBuffer->dataLenInBytes;

        for(counter = 0;
            (counter < maxDataSize) && (0 == dataSizeInBytes);
            counter++)
        {
            if(pBuffer->pData[counter])
            {
                dataSizeInBytes = maxDataSize - counter;
            }
        }
    }
    return dataSizeInBytes;
}


/**
 *******************************************************************************
 * @ingroup LacAsymLn
 *      Large Number Modular Exponentation internal callback function
 ******************************************************************************/
STATIC
void
LacLnModExpCallback(
    CpaStatus status,
    CpaBoolean pass,
    CpaInstanceHandle instanceHandle,
    lac_pke_op_cb_data_t *pCbData)
{
    CpaCyGenFlatBufCbFunc pCb = NULL;
    CpaCyLnModExpOpData *pOpData = NULL;
    void *pCallbackTag = NULL;
    CpaFlatBuffer *pResult = NULL;

    /* retrieve data from the callback data structure */
    LAC_ASSERT_NOT_NULL(pCbData);
    pCb = (CpaCyGenFlatBufCbFunc) LAC_CONST_PTR_CAST(pCbData->pClientCb);
    pOpData = (CpaCyLnModExpOpData *) 
                  LAC_CONST_PTR_CAST(pCbData->pClientOpData);
    pCallbackTag = pCbData->pCallbackTag;
    pResult = pCbData->pOutputData1;

    LAC_ASSERT_NOT_NULL(pCb);
    LAC_ASSERT_NOT_NULL(pOpData);
    LAC_ASSERT_NOT_NULL(pResult);

    /* pass flag is not used here */

    /* increment stats */
    if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.ln.istat)
    {
        if (CPA_STATUS_SUCCESS == status)
        {
            LAC_LN_STAT_INC(numLnModExpCompleted, instanceHandle);
        }
        else
        {
            LAC_LN_STAT_INC(numLnModExpCompletedErrors, instanceHandle);
        }
    }

    /* invoke the user callback */
    pCb(pCallbackTag, status, pOpData, pResult);
}

#ifdef ICP_PARAM_CHECK
/**
 *******************************************************************************
 * @ingroup LacAsymLn
 *      Ln ModExp parameter check
 ******************************************************************************/
STATIC
CpaStatus
LacLnModExpParameterCheck(
    const CpaCyGenFlatBufCbFunc pCb,
    CpaCyLnModExpOpData *pOpData,
    CpaFlatBuffer *pResult)
{

    CpaStatus status = CPA_STATUS_SUCCESS;

    /* ensure LAC is initialized - return error if not */
    LAC_INITIALISED_CHECK();

    /* check for valid callback function pointer */
    LAC_CHECK_NULL_PARAM(pCb);
    /* check for null Operational Data parameters */
    LAC_CHECK_NULL_PARAM(pOpData);
    /* check for null result pointer */
    LAC_CHECK_NULL_PARAM(pResult);

    /* check for null argument parameters */
    LAC_CHECK_FLAT_BUFFER(&pOpData->base);
    LAC_CHECK_ZERO_SIZE(&pOpData->base);
    LAC_CHECK_FLAT_BUFFER(&pOpData->exponent);
    LAC_CHECK_ZERO_SIZE(&pOpData->exponent);
    LAC_CHECK_FLAT_BUFFER(&pOpData->modulus);
    LAC_CHECK_ZERO_SIZE(&pOpData->modulus);
    LAC_CHECK_FLAT_BUFFER(pResult);
    LAC_CHECK_ZERO_SIZE(pResult);

    /* Zero modulus is an invalid parameter */
    LAC_CHECK_NON_ZERO_PARAM(&pOpData->modulus);

    return status;
}
#endif

/**
 ***************************************************************************
 * @ingroup LacAsymLn
 *      Large Number Modular Exponentation synchronous function
 ***************************************************************************/
STATIC CpaStatus
LacLnModExpSyn(const CpaInstanceHandle instanceHandle,
        const CpaCyGenFlatBufCbFunc pLnModExpCb,
        void *pCallbackTag,
        const CpaCyLnModExpOpData *pLnModExpOpData,
        CpaFlatBuffer *pResult)
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
        status = cpaCyLnModExp(instanceHandle, 
                    LacSync_GenFlatBufCb, pSyncCallbackData, 
                    pLnModExpOpData, pResult);
    }
    else
    {
        if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.ln.istat)
        {
            LAC_LN_STAT_INC(numLnModExpRequestErrors, instanceHandle);
        }
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
            if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.ln.istat)
            {
                LAC_LN_STAT_INC(numLnModExpCompletedErrors,
                        instanceHandle);
            }
            status = wCbStatus;
        }
    }
    LacSync_DestroySyncCookie(&pSyncCallbackData);
    return status;
}

/**
 *******************************************************************************
 * @ingroup LacAsymLn
 *      Large Number Modular Exponentation API function
 ******************************************************************************/
CpaStatus
cpaCyLnModExp(const CpaInstanceHandle instanceHandle,
        const CpaCyGenFlatBufCbFunc pLnModExpCb,
        void *pCallbackTag,
        const CpaCyLnModExpOpData *pLnModExpOpData,
        CpaFlatBuffer *pResult)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    Cpa32U functionalityId = LAC_PKE_INVALID_FUNC_ID;
    Cpa32U index = LAC_PKE_INVALID_INDEX;
    Cpa32U dataOperationSize = 0;

    Cpa32U dataLenCommon = 0;
    Cpa32U dataLenBase = 0;
    Cpa32U dataLenExponent = 0;
    Cpa32U dataLenModulus = 0;
    Cpa32U dataLenResult = 0;

    icp_qat_fw_mmp_input_param_t inArgList = { .flat_array = {0} };
    icp_qat_fw_mmp_output_param_t outArgList= { .flat_array = {0} };

    /* Holding the calculated size of the input/output parameters */
    Cpa32U inArgSizeList[LAC_MAX_MMP_INPUT_PARAMS] = {0};
    Cpa32U outArgSizeList[LAC_MAX_MMP_OUTPUT_PARAMS] = {0};

    /* Data that will be passed back in call back function - opaque data */
    lac_pke_op_cb_data_t lnModExpData = {0};

    /* Check if the API has been called in sync mode */
    if (NULL == pLnModExpCb)
    {
        return LacLnModExpSyn(instanceHandle, pLnModExpCb,
                pCallbackTag, pLnModExpOpData, pResult);
    }

#ifdef ICP_PARAM_CHECK
    /* check for valid acceleration handle */
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);

    /* check that the input parameters are valid */
    status = LacLnModExpParameterCheck(pLnModExpCb, 
                 LAC_CONST_PTR_CAST(pLnModExpOpData), pResult);
#endif

    if (CPA_STATUS_SUCCESS == status)
    {
        /* calculate the actual data size of the input parameters in bytes */
        dataLenBase = LacGetBufferDataSizeInBytes(&pLnModExpOpData->base);
        dataLenExponent = 
            LacGetBufferDataSizeInBytes(&pLnModExpOpData->exponent);
        dataLenModulus = LacGetBufferDataSizeInBytes(&pLnModExpOpData->modulus);
        dataLenResult = pResult->dataLenInBytes;
    
        /* the Result buffer size has to be at least the size of the Modulus */
        if (dataLenResult < dataLenModulus)
        {
            LAC_INVALID_PARAM_LOG("Result buffer size must be >= Modulus size");
            status = CPA_STATUS_INVALID_PARAM;
        }
    }
    if (CPA_STATUS_SUCCESS == status)
    {        
        /* calculate the smallest common bit size for the three
        *  imput parameters */
        dataLenCommon = (dataLenBase > dataLenExponent) ? 
            dataLenBase : dataLenExponent;
        dataLenCommon = (dataLenCommon > dataLenModulus) ?
            dataLenCommon : dataLenModulus;
        dataLenCommon *= LAC_NUM_BITS_IN_BYTE;

        /* One row in the maping table contains operation size and matching
        *  function Id. First, calculate the index of that row using the common
        *  data length */
        index = LacPke_GetIndex_VariableSize(
            dataLenCommon,
            LAC_MATHS_MODEXP_SIZE_ID_MAP,
            LAC_ARRAY_LEN(LAC_MATHS_MODEXP_SIZE_ID_MAP));
        if (LAC_PKE_INVALID_INDEX == index)
        {
            LAC_LOG_ERROR("The input data size is not supported");
            status = CPA_STATUS_FAIL;
        }
    }

    if (CPA_STATUS_SUCCESS == status)
    { 
        /* the input/output parameters have to be padded to the operation size
        *  for QAT processing so assign the new size in bytes to each paremeter
        *  before the request has been sent out to QAT */
        
        dataOperationSize =
            LAC_MATHS_MODEXP_SIZE_ID_MAP[index][LAC_PKE_SIZE_COLUMN]/
            LAC_NUM_BITS_IN_BYTE;

          inArgSizeList[0] = dataOperationSize;
          inArgSizeList[1] = dataOperationSize;
          inArgSizeList[2] = dataOperationSize;
          outArgSizeList[0] = dataOperationSize;

        /* get the functionality ID that matches operation size */
        functionalityId =
            LAC_MATHS_MODEXP_SIZE_ID_MAP[index][LAC_PKE_ID_COLUMN];

        /* preserve user parameters for when our Call Back
        *  function kicks in, after sending the head request */

        lnModExpData.pClientCb = pLnModExpCb;
        lnModExpData.pCallbackTag = pCallbackTag;
        lnModExpData.pClientOpData = pLnModExpOpData;
        lnModExpData.pOpaqueData = NULL;
        lnModExpData.pOutputData1 = pResult;


            
        /* User alocated some memory for the result. Initialise it with zeros
        *  beforehand  */
        LAC_OS_BZERO(pResult->pData, dataLenResult);

        
        /* populate input/output parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(inArgList.maths_modexp_l512.g,
            &pLnModExpOpData->base);
        LAC_MEM_SHARED_WRITE_FROM_PTR(inArgList.maths_modexp_l512.e,
            &pLnModExpOpData->exponent);
        LAC_MEM_SHARED_WRITE_FROM_PTR(inArgList.maths_modexp_l512.m,
            &pLnModExpOpData->modulus);
        LAC_MEM_SHARED_WRITE_FROM_PTR(outArgList.maths_modexp_l512.r,
            pResult);            
            
        /* Send a PKE request */
        status = LacPke_SendSingleRequest(functionalityId,
            inArgSizeList, outArgSizeList,        
            &inArgList, &outArgList,
            LacLnModExpCallback, &lnModExpData, instanceHandle);
    }

    /* update stats. */
    if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.ln.istat)
    {
        if (CPA_STATUS_SUCCESS == status)
        {
            /* increment stats:
            *  Total number of LN ModExp test requested operations */
            LAC_LN_STAT_INC(numLnModExpRequests, instanceHandle);
        }
        else
        {
            /* on failure increment stats:
            * Total number of LN ModExp test errors recorded */
            LAC_LN_STAT_INC(numLnModExpRequestErrors, instanceHandle);
        }
    }

    return status;
}

/**
 *******************************************************************************
 * @ingroup LacAsymLn
 *      Large Number Modular Inversion internal callback function
 ******************************************************************************/
STATIC
void
LacLnModInvCallback(
    CpaStatus status,
    CpaBoolean pass,
    CpaInstanceHandle instanceHandle,
    lac_pke_op_cb_data_t *pCbData)
{
    CpaCyGenFlatBufCbFunc pCb = NULL;
    CpaCyLnModInvOpData *pOpData = NULL;
    void *pCallbackTag = NULL;
    CpaFlatBuffer *pResult = NULL;

    /* retrieve data from the callback data structure */
    LAC_ASSERT_NOT_NULL(pCbData);
    pCb = (CpaCyGenFlatBufCbFunc) LAC_CONST_PTR_CAST(pCbData->pClientCb);
    pOpData = (CpaCyLnModInvOpData *) 
                  LAC_CONST_PTR_CAST(pCbData->pClientOpData);
    pCallbackTag = pCbData->pCallbackTag;
    pResult = pCbData->pOutputData1;

    LAC_ASSERT_NOT_NULL(pCb);
    LAC_ASSERT_NOT_NULL(pOpData);
    LAC_ASSERT_NOT_NULL(pResult);

    /* pass flag is not used here */

    /* increment stats */
    if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.ln.istat)
    {
        if (CPA_STATUS_SUCCESS == status)
        {
            LAC_LN_STAT_INC(numLnModInvCompleted, instanceHandle);
        }
        else
        {
            LAC_LN_STAT_INC(numLnModInvCompletedErrors, instanceHandle);
        }
    }

    /* invoke the user callback */
    pCb(pCallbackTag, status, pOpData, pResult);
}

#ifdef ICP_PARAM_CHECK
/**
 *******************************************************************************
 * @ingroup LacAsymLn
 *      Ln ModInv parameter check
 ******************************************************************************/
STATIC
CpaStatus
LacLnModInvParameterCheck(
    const CpaCyGenFlatBufCbFunc pCb,
    CpaCyLnModInvOpData *pOpData,
    CpaFlatBuffer *pResult)
{

    CpaStatus status = CPA_STATUS_SUCCESS;

    /* ensure LAC is initialized - return error if not */
    LAC_INITIALISED_CHECK();

    /* check for valid callback function pointer */
    LAC_CHECK_NULL_PARAM(pCb);
    /* check for null Operational Data parameters */
    LAC_CHECK_NULL_PARAM(pOpData);
    /* check for null result pointer */
    LAC_CHECK_NULL_PARAM(pResult);

    /* check for null argument parameters */

    LAC_CHECK_FLAT_BUFFER(&pOpData->A);
    LAC_CHECK_ZERO_SIZE(&pOpData->A);
    LAC_CHECK_FLAT_BUFFER(&pOpData->B);
    LAC_CHECK_ZERO_SIZE(&pOpData->B);
    LAC_CHECK_FLAT_BUFFER(pResult);
    LAC_CHECK_ZERO_SIZE(pResult);
    
    /* Input parameters should not be equal to zero */
    LAC_CHECK_NON_ZERO_PARAM(&pOpData->A);
    LAC_CHECK_NON_ZERO_PARAM(&pOpData->B);
    
    /* the input parameters are invalid if both modulus (B) and the value to be
    *  inverted (A) are even (LSB is not set)*/   


    if(LAC_CHECK_EVEN_PARAM(&pOpData->A) && LAC_CHECK_EVEN_PARAM(&pOpData->B))
    {
        LAC_INVALID_PARAM_LOG("Both modulus and value to invert are even");
        status = CPA_STATUS_INVALID_PARAM;
    }
  
    return status;
}
#endif

/**
 ***************************************************************************
 * @ingroup LacAsymLn
 *      Large Number Modular Inversion synchronous function
 ***************************************************************************/
STATIC CpaStatus
LacLnModInvSyn(const CpaInstanceHandle instanceHandle,
        const CpaCyGenFlatBufCbFunc pLnModInvCb,
        void *pCallbackTag,
        const CpaCyLnModInvOpData *pLnModInvOpData,
        CpaFlatBuffer *pResult)
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
        status = cpaCyLnModInv(instanceHandle, 
                    LacSync_GenFlatBufCb, pSyncCallbackData, 
                    pLnModInvOpData, pResult);
    }
    else
    {
        if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.ln.istat)
        {
            LAC_LN_STAT_INC(numLnModInvRequestErrors, instanceHandle);
        }
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
            if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.ln.istat)
            {
                LAC_LN_STAT_INC(numLnModInvCompletedErrors,
                        instanceHandle);
            }
            status = wCbStatus;
        }
    }
    LacSync_DestroySyncCookie(&pSyncCallbackData);
    return status;
}

/**
 *******************************************************************************
 * @ingroup LacAsymLn
 *      Large Number Modular Inversion API function
 ******************************************************************************/
CpaStatus
cpaCyLnModInv(const CpaInstanceHandle instanceHandle,
        const CpaCyGenFlatBufCbFunc pLnModInvCb,
        void *pCallbackTag,
        const CpaCyLnModInvOpData *pLnModInvOpData,
        CpaFlatBuffer *pResult)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    Cpa32U functionalityId = LAC_PKE_INVALID_FUNC_ID;
    Cpa32U index = LAC_PKE_INVALID_INDEX;
    Cpa32U dataOperationSize = 0;

    Cpa32U dataLenCommon = 0;
    Cpa32U dataLenA = 0;
    Cpa32U dataLenB = 0;
    Cpa32U dataLenResult = 0;

    icp_qat_fw_mmp_input_param_t inArgList = { .flat_array = {0} };
    icp_qat_fw_mmp_output_param_t outArgList= { .flat_array = {0} };
    
    /* Holding the calculated size of the input/output parameters */
    Cpa32U inArgSizeList[LAC_MAX_MMP_INPUT_PARAMS] = {0};
    Cpa32U outArgSizeList[LAC_MAX_MMP_OUTPUT_PARAMS] = {0};

    /* Data that will be passed back in call back function - opaque data */
    lac_pke_op_cb_data_t lnModInvData = {0};

    /* Check if the API has been called in sync mode */
    if (NULL == pLnModInvCb)
    {
        return LacLnModInvSyn(instanceHandle, pLnModInvCb,
                pCallbackTag, pLnModInvOpData, pResult);
    }

#ifdef ICP_PARAM_CHECK
    /* check for valid acceleration handle */
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);

    /* check that the input parameters are valid */
    status = LacLnModInvParameterCheck(pLnModInvCb, 
                 LAC_CONST_PTR_CAST(pLnModInvOpData), pResult);
#endif
    
    if (CPA_STATUS_SUCCESS == status)
    {
        /* calculate the actual data size of the input parameters in bytes */
        dataLenA = LacGetBufferDataSizeInBytes(&pLnModInvOpData->A);
        dataLenB = LacGetBufferDataSizeInBytes(&pLnModInvOpData->B);
        dataLenResult = pResult->dataLenInBytes;

    
        /* the Result buffer size has to be at least the size of the Modulus */
        if (dataLenResult < dataLenB)
        {
            LAC_INVALID_PARAM_LOG("Result buffer size less then Modulus size");
            status = CPA_STATUS_INVALID_PARAM;
        }
    }
    if (CPA_STATUS_SUCCESS == status)
    {       

        /* calculate the smallest common bit size for the two input params */
        dataLenCommon = (dataLenA > dataLenB) ? dataLenA : dataLenB;
        dataLenCommon *= LAC_NUM_BITS_IN_BYTE;

        /* Both ODD and EVEN maping table contain the same operation sizes.
        *  data length. The ODD table is used here */
        index = LacPke_GetIndex_VariableSize(
            dataLenCommon,
            LAC_MATHS_MODINV_ODD_SIZE_ID_MAP,
            LAC_ARRAY_LEN(LAC_MATHS_MODINV_ODD_SIZE_ID_MAP));
        if (LAC_PKE_INVALID_INDEX == index)
        {
            LAC_LOG_ERROR("The input data size is not supported");
            status = CPA_STATUS_FAIL;
        }
    }
 
    if (CPA_STATUS_SUCCESS == status)
    {
        /* the input/output parameters have to be padded to the operation size
        *  for QAT processing so assign the new size in bytes to each paremeter
        *  before the request has been sent out to QAT */
        
        /* Both ODD and EVEN maping tables can give the right oparation size
        *  that corresponds to the index. The ODD table is used here. */
        dataOperationSize =
            LAC_MATHS_MODINV_ODD_SIZE_ID_MAP[index][LAC_PKE_SIZE_COLUMN]/
            LAC_NUM_BITS_IN_BYTE;
            
          inArgSizeList[0] = dataOperationSize;
          inArgSizeList[1] = dataOperationSize;
          outArgSizeList[0] = dataOperationSize;

        /* get functionality ID based on odd/even character of the parameters -
        *  we already confirmed that at least one paremeter is odd, otherwise
        *  we wouldn't reach this point */



        if (LAC_CHECK_ODD_PARAM(&pLnModInvOpData->B))
        {
            /* if pA is odd/even and pB paremeter is odd, look into ODD 
            *  functinality ID pool */
        functionalityId =
            LAC_MATHS_MODINV_ODD_SIZE_ID_MAP[index][LAC_PKE_ID_COLUMN];
        }
        else
        {
            /* if pA is odd and pB is even (already confirmed that at least 
            *  one is odd), look into EVEN functinality ID pool */
        functionalityId =
            LAC_MATHS_MODINV_EVEN_SIZE_ID_MAP[index][LAC_PKE_ID_COLUMN];
        }

        /* preserve user parameters for when our Call Back
        *  function kicks in, after sending the head request */

        lnModInvData.pClientCb = pLnModInvCb;
        lnModInvData.pCallbackTag = pCallbackTag;
        lnModInvData.pClientOpData = pLnModInvOpData;
        lnModInvData.pOpaqueData = NULL;
        lnModInvData.pOutputData1 = pResult;


        /* User alocated some memory for the result. Initialise it with zeros
        *  beforehand  */
        LAC_OS_BZERO(pResult->pData, dataLenResult);

        /* populate input/output parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(inArgList.maths_modinv_odd_l128.a,
            &pLnModInvOpData->A);
        LAC_MEM_SHARED_WRITE_FROM_PTR(inArgList.maths_modinv_odd_l128.b,
            &pLnModInvOpData->B);
        LAC_MEM_SHARED_WRITE_FROM_PTR(outArgList.maths_modinv_odd_l128.c,
            pResult);            
            
        /* Send a PKE request */
        status = LacPke_SendSingleRequest(functionalityId,
            inArgSizeList, outArgSizeList,
            &inArgList, &outArgList,
            LacLnModInvCallback, &lnModInvData, instanceHandle);
    }

    /* update stats. */
    if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.ln.istat)
    {
        if (CPA_STATUS_SUCCESS == status)
        {
            /* increment stats:
            *  Total number of LN ModExp test requested operations */
            LAC_LN_STAT_INC(numLnModInvRequests, instanceHandle);
        }
        else
        {
            /* on failure increment stats:
            * Total number of LN ModExp test errors recorded */
            LAC_LN_STAT_INC(numLnModInvRequestErrors, instanceHandle);
        }
    }

    return status;
}

/**
 *******************************************************************************
 * @ingroup LacAsymLn
 *      Ln Statistics Query API function
 ******************************************************************************/
CpaStatus
cpaCyLnStatsQuery(
    CpaInstanceHandle instanceHandle,
    CpaCyLnStats *pLnStats)
{
    /* ensure LAC is initialized - return error if not */
    LAC_INITIALISED_CHECK();

    if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.ln.istat)
    {
        /* check for valid acceleration handle */
        LAC_CHECK_INSTANCE_HANDLE(instanceHandle);

        /* check for null parameters */
        LAC_CHECK_NULL_PARAM(pLnStats);

        /* get stats into user supplied stats structure */
        LAC_LN_STATS_GET(*pLnStats);
    }
    else
    {
        return CPA_STATUS_RESOURCE;
    }

    return CPA_STATUS_SUCCESS;
}

/**
 *******************************************************************************
 * @ingroup LacAsymLn
 *      Ln Initialization function
 ******************************************************************************/
CpaStatus
LacLn_Init(void)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    /* initialize stats to zero */
    LAC_LN_STATS_INIT();

    return status;
}

/**
 *******************************************************************************
 * @ingroup LacAsymLn
 *      Ln Shutdown function
 ******************************************************************************/
CpaStatus
LacLn_Shutdown(void)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
        
    return status;
}

/**
 *******************************************************************************
 * @ingroup LacAsymLn
 *      Ln Stats Show function
 ******************************************************************************/
void
LacLn_StatsShow(void)
{
    CpaCyLnStats lnStats = {0};
    CpaInstanceHandle instanceHandle = CPA_INSTANCE_HANDLE_SINGLE;

    /* retrieve the stats */
    (void)cpaCyLnStatsQuery(instanceHandle, &lnStats);

    /* log the stats to the standard output */

    /* Engine Info */
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
        IX_OSAL_LOG_DEV_STDOUT,
        SEPARATOR           
        BORDER " LN ModExp/ModInv Stats                     " BORDER "\n"
        SEPARATOR,
        0, 0, 0, 0, 0, 0);


    /* Large Number Modular Exponentationstats operations stats */
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
        IX_OSAL_LOG_DEV_STDOUT,
        BORDER " LN ModEXP successful requests:  %10u " BORDER "\n"
        BORDER " LN ModEXP requests with error:  %10u " BORDER "\n"
        BORDER " LN ModEXP completed operations: %10u " BORDER "\n"
        BORDER " LN ModEXP not completed-errors: %10u " BORDER "\n"
        SEPARATOR,
        lnStats.numLnModExpRequests,
        lnStats.numLnModExpRequestErrors,
        lnStats.numLnModExpCompleted,
        lnStats.numLnModExpCompletedErrors,
        0,0);

    /*  Large Number Modular Inversion operations stats */
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
        IX_OSAL_LOG_DEV_STDOUT,
        BORDER " LN ModINV successful requests:  %10u " BORDER "\n"
        BORDER " LN ModINV requests with error:  %10u " BORDER "\n"
        BORDER " LN ModINV completed operations: %10u " BORDER "\n"
        BORDER " LN ModINV not completed-errors: %10u " BORDER "\n"
        SEPARATOR,
        lnStats.numLnModInvRequests,
        lnStats.numLnModInvRequestErrors,
        lnStats.numLnModInvCompleted,
        lnStats.numLnModInvCompletedErrors,
        0,0);


}
