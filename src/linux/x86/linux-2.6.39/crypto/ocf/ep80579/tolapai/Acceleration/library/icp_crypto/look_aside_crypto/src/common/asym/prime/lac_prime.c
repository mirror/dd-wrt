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
 * @file lac_prime.c Prime API Implementation
 *
 * @ingroup Lac_Prime
 *
 * @description
 *      This file contains the implementation of Prime functions
 *
 ***************************************************************************/

/*
********************************************************************************
* Include public/global header files
********************************************************************************
*/

/* Include API files */
#include "cpa.h"
#include "cpa_cy_prime.h"
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
#include "lac_prime.h"
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

#define LAC_PRIME_NUM_STATS (sizeof(CpaCyPrimeStats) / sizeof(Cpa32U))
/**<
 * number of Prime stats */

STATIC IxOsalAtomic lacPrimeStats[LAC_PRIME_NUM_STATS];
/**<
 * array of atomics for Prime stats */

/**<
 * macro to initialize all Prime stats (stored in internal array of atomics) */
#define LAC_PRIME_STATS_INIT()                                              \
do {                                                                        \
    Cpa32U i;                                                               \
                                                                            \
    for (i = 0; i < LAC_PRIME_NUM_STATS; i++)                               \
    {                                                                       \
        ixOsalAtomicSet(0, &lacPrimeStats[i]);                              \
    }                                                                       \
} while (0)

/**<
 * macro to increment a Prime stat (derives offset into array of atomics) */
#define LAC_PRIME_STAT_INC(statistic, instanceHandle)                       \
do {                                                                        \
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);                              \
                                                                            \
    ixOsalAtomicInc(                                                        \
        &lacPrimeStats[                                                     \
            offsetof(CpaCyPrimeStats, statistic) / sizeof(Cpa32U)]);\
} while (0)

/**<
 * macro to get all Prime stats (from internal array of atomics) into user
 * supplied structure CpaCyPrimeStats pointed by primeStats pointer */
#define LAC_PRIME_STATS_GET(primeStats)                                     \
do {                                                                        \
    Cpa32U i;                                                               \
                                                                            \
    for (i = 0; i < LAC_PRIME_NUM_STATS; i++)                               \
    {                                                                       \
        ((Cpa32U *)&(primeStats))[i] = ixOsalAtomicGet(&lacPrimeStats[i]);  \
    }                                                                       \
} while (0)


#define LAC_PRIME_MAX_MILLER_RABIN_ROUNDS    50
#define MAX_MR (LAC_PRIME_MAX_MILLER_RABIN_ROUNDS)
/**<
 * define max for the number of Miller-Rabin rounds */

#define MILLER_RABIN_START_IDX (0)
#define MILLER_RABIN_END_IDX (MILLER_RABIN_START_IDX + MAX_MR -1)
/**<
 * indexing for pBuffInputMillerRabin */


STATIC const Cpa8U lacPrimePsp1Data[] = {
    0x02, 0xc8, 0x5f, 0xf8, 0x70, 0xf2, 0x4b, 0xe8,
    0x0f, 0x62, 0xb1, 0xba, 0x6c, 0x20, 0xbd, 0x72,
    0xb8, 0x37, 0xef, 0xdf, 0x12, 0x12, 0x06, 0xd8,
    0x7d, 0xb5, 0x6b, 0x7d, 0x69, 0xfa, 0x4c, 0x02,
    0x1c, 0x10, 0x7c, 0x3c, 0xa2, 0x06, 0xfe, 0x8f,
    0xa7, 0x08, 0x0e, 0xf5, 0x76, 0xef, 0xfc, 0x82,
    0xf9, 0xb1, 0x0f, 0x57, 0x50, 0x65, 0x6b, 0x77,
    0x94, 0xb1, 0x6a, 0xfd, 0x70, 0x99, 0x6e, 0x91,
    0xae, 0xf6, 0xe0, 0xad, 0x15, 0xe9, 0x1b, 0x07,
    0x1a, 0xc9, 0xb2, 0x4d, 0x98, 0xb2, 0x33, 0xad,
    0x86, 0xee, 0x05, 0x55, 0x18, 0xe5, 0x8e, 0x56,
    0x63, 0x8e, 0xf1, 0x8b, 0xac, 0x5c, 0x74, 0xcb,
    0x35, 0xbb, 0xb6, 0xe5, 0xda, 0xe2, 0x78, 0x3d,
    0xd1, 0xc0, 0xce, 0x7d, 0xec, 0x4f, 0xc7, 0x0e,
    0x51, 0x86, 0xd4, 0x11, 0xdf, 0x36, 0x36, 0x8f,
    0x06, 0x1a, 0xa3, 0x60, 0x11, 0xf3, 0x01, 0x79
};
/**<
 * Product of small primes #1 = 3 * 5 * 7 * 11 * ... * 739 (1024-bit) */

STATIC CpaFlatBuffer lacPrimePsp1 = { .pData = NULL, .dataLenInBytes = 0 };
/**<
 * Flat buffer for product of small primes #1.  Note pData can't be assigned
 * to the lacPrimePsp1Data structure above, as the pData buffer must be
 * allocated in shared memory and be properly aligned - instead it is allocated
 * and populated during run-time Initialization. */

static const Cpa32U LAC_GCD_SIZE_ID_MAP[][LAC_PKE_NUM_COLUMNS] =
{
    {LAC_160_BITS, PKE_GCD_PT_192},
    {LAC_192_BITS, PKE_GCD_PT_192}, 
    {LAC_256_BITS, PKE_GCD_PT_256},
    {LAC_384_BITS, PKE_GCD_PT_384},
    {LAC_512_BITS, PKE_GCD_PT_512},
    {LAC_768_BITS, PKE_GCD_PT_768},
    {LAC_1024_BITS, PKE_GCD_PT_1024},
    {LAC_1536_BITS, PKE_GCD_PT_1536},
    {LAC_2048_BITS, PKE_GCD_PT_2048},
    {LAC_3072_BITS, PKE_GCD_PT_3072},
    {LAC_4096_BITS, PKE_GCD_PT_4096}
};
/**<
 * Maps between operation sizes and GCD PKE function ids */

static const Cpa32U LAC_FERMAT_SIZE_ID_MAP[][LAC_PKE_NUM_COLUMNS] =
{
    {LAC_160_BITS, PKE_FERMAT_PT_160},
    {LAC_512_BITS, PKE_FERMAT_PT_512},
    {LAC_768_BITS, PKE_FERMAT_PT_768},
    {LAC_1024_BITS, PKE_FERMAT_PT_1024},
    {LAC_1536_BITS, PKE_FERMAT_PT_1536},
    {LAC_2048_BITS, PKE_FERMAT_PT_2048},
    {LAC_3072_BITS, PKE_FERMAT_PT_3072},
    {LAC_4096_BITS, PKE_FERMAT_PT_4096}
};
/**<
 * Maps between operation sizes and Fermat PKE function ids */

static const Cpa32U LAC_LUCAS_SIZE_ID_MAP[][LAC_PKE_NUM_COLUMNS] =
{
    {LAC_160_BITS, PKE_LUCAS_PT_160},
    {LAC_512_BITS, PKE_LUCAS_PT_512},
    {LAC_768_BITS, PKE_LUCAS_PT_768},
    {LAC_1024_BITS, PKE_LUCAS_PT_1024},
    {LAC_1536_BITS, PKE_LUCAS_PT_1536},
    {LAC_2048_BITS, PKE_LUCAS_PT_2048},
    {LAC_3072_BITS, PKE_LUCAS_PT_3072},
    {LAC_4096_BITS, PKE_LUCAS_PT_4096}
};
/**<
 * Maps between operation sizes and Lucas PKE function ids */

static const Cpa32U LAC_MR_SIZE_ID_MAP[][LAC_PKE_NUM_COLUMNS] =
{
    {LAC_160_BITS, PKE_MR_PT_160},
    {LAC_512_BITS, PKE_MR_PT_512},
    {LAC_768_BITS, PKE_MR_PT_768},
    {LAC_1024_BITS, PKE_MR_PT_1024},
    {LAC_1536_BITS, PKE_MR_PT_1536},
    {LAC_2048_BITS, PKE_MR_PT_2048},
    {LAC_3072_BITS, PKE_MR_PT_3072},
    {LAC_4096_BITS, PKE_MR_PT_4096}
};
/**<
 * Maps between operation sizes and Miller-Rabin PKE function ids */

/**<
 * Enum for Prime testing */

static lac_memory_pool_id_t lac_prime_pool = LAC_MEM_POOL_INIT_POOL_ID;
/**<
 * Identifier for pool of aligned buffers */

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
 * @ingroup Lac_Prime
 *      Prime Test internal callback function
 ******************************************************************************/
void
LacPrime_TestCallback(
    CpaStatus status,
    CpaBoolean pass,
    CpaInstanceHandle instanceHandle,
    lac_pke_op_cb_data_t *pCbData)
{
    CpaCyPrimeTestCbFunc pCb = NULL;
    CpaCyPrimeTestOpData *pOpData = NULL;
    CpaFlatBuffer * pBuffInMillerRabin = NULL;
    void *pCallbackTag = NULL;

    /* retrieve data from the callback data structure */
    LAC_ASSERT_NOT_NULL(pCbData);
    pCb = (CpaCyPrimeTestCbFunc) LAC_CONST_PTR_CAST(pCbData->pClientCb);
    pOpData = (CpaCyPrimeTestOpData *)
                  LAC_CONST_PTR_CAST(pCbData->pClientOpData);
    pBuffInMillerRabin = (CpaFlatBuffer *)(pCbData->pOpaqueData);
    pCallbackTag = pCbData->pCallbackTag;

    LAC_ASSERT_NOT_NULL(pCb);
    LAC_ASSERT_NOT_NULL(pOpData);
    LAC_ASSERT_NOT_NULL(pBuffInMillerRabin);

    /* free the array of the input/output flat buffers */
    if (NULL != pBuffInMillerRabin)
    {
        Lac_MemPoolEntryFree(pBuffInMillerRabin);
    }

    if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.prime.istat)
    {
        /* increment stats */
        if (CPA_STATUS_SUCCESS == status)
        {
            LAC_PRIME_STAT_INC(numPrimeTestCompleted, instanceHandle);
            if (CPA_FALSE == pass)
            {
                LAC_PRIME_STAT_INC(numPrimeTestFailures, instanceHandle);
            }
        }
        else
        {
            LAC_PRIME_STAT_INC(numPrimeTestCompletedErrors, instanceHandle);
        }
    }
    /* invoke the user callback */
    pCb(pCallbackTag, status, pOpData, pass);
}


/**
 *******************************************************************************
 * @ingroup Lac_Prime
 *      Prime Get Function ID function
 ******************************************************************************/
Cpa32U
LacPrime_GetFuncID(
    lac_prime_test_t testId,
    Cpa32U dataLenInBits)
{
    Cpa32U functionalityId = LAC_PKE_INVALID_FUNC_ID;

    /*
    * get functionality ID for GCD request
    */
    if (LAC_PRIME_GCD == testId)
    {
        functionalityId = LacPke_GetMmpId(
            dataLenInBits,
            LAC_GCD_SIZE_ID_MAP,
            LAC_ARRAY_LEN(LAC_GCD_SIZE_ID_MAP));
    }
    /*
    * get functionality ID for Fermat request
    */
    if (LAC_PRIME_FERMAT == testId)
    {
        functionalityId = LacPke_GetMmpId(
            dataLenInBits,
            LAC_FERMAT_SIZE_ID_MAP,
            LAC_ARRAY_LEN(LAC_FERMAT_SIZE_ID_MAP));
    }
    /*
    * get functionality ID for Miller-Rabin request
    */
    else if (LAC_PRIME_MILLER_RABIN == testId)
    {
        functionalityId = LacPke_GetMmpId(
            dataLenInBits,
            LAC_MR_SIZE_ID_MAP,
            LAC_ARRAY_LEN(LAC_MR_SIZE_ID_MAP));
    }
    /*
    * get functionality ID for Lucas request
    */
    else if (LAC_PRIME_LUCAS == testId)
    {
        functionalityId = LacPke_GetMmpId(
            dataLenInBits,
            LAC_LUCAS_SIZE_ID_MAP,
            LAC_ARRAY_LEN(LAC_LUCAS_SIZE_ID_MAP));
    }
    return functionalityId;
}

 /**
 *******************************************************************************
 * @ingroup Lac_Prime
 *      Populate Prime Input and Output Parameter function
 ******************************************************************************/
void
LacPrime_PopulateParam(
    lac_prime_test_t testId,
    icp_qat_fw_mmp_input_param_t *pIn,
    icp_qat_fw_mmp_output_param_t *pOut,
    CpaCyPrimeTestOpData *pOpData,
    const CpaFlatBuffer *pInputMillerRabinBuffer,
    const CpaFlatBuffer *pGcdProductOfSmallPrimes)
{
    /*
    * populate input/output parameters for GCD request
    * using mmp_gcd_pt_192 as generic structure
    */
    if (LAC_PRIME_GCD == testId)
    {
        LAC_MEM_SHARED_WRITE_FROM_PTR(pIn->mmp_gcd_pt_192.m,
            &pOpData->primeCandidate);
        LAC_MEM_SHARED_WRITE_FROM_PTR(pIn->mmp_gcd_pt_192.psp,
            pGcdProductOfSmallPrimes);
    }
    /*
    * populate input/output parameters for Fermat request
    * using mmp_fermat_pt_160 as generic structure
    */
    if (LAC_PRIME_FERMAT == testId)
    {
        LAC_MEM_SHARED_WRITE_FROM_PTR(pIn->mmp_fermat_pt_160.m,
            &pOpData->primeCandidate);
    }
    /*
    * populate input/output parameters Miller-Rabin request
    * using mmp_mr_pt_160 as generic structure
    */
    else if (LAC_PRIME_MILLER_RABIN == testId)
    {
        /* populate input/output parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(pIn->mmp_mr_pt_160.x,
            pInputMillerRabinBuffer);
        LAC_MEM_SHARED_WRITE_FROM_PTR(pIn->mmp_mr_pt_160.m,
            &pOpData->primeCandidate);
    }
    /*
    * populate input/output parameters for Lucas request
    * using mmp_lucas_pt_160 as generic structure
    */
    else if (LAC_PRIME_LUCAS == testId)
    {
        LAC_MEM_SHARED_WRITE_FROM_PTR(pIn->mmp_lucas_pt_160.m,
            &pOpData->primeCandidate);
    }
}


#ifdef ICP_PARAM_CHECK
/**
 *******************************************************************************
 * @ingroup Lac_Prime
 *      Prime Test parameter check
 ******************************************************************************/
CpaStatus
LacPrime_ParameterCheck(
    CpaCyPrimeTestCbFunc pCb,
    CpaCyPrimeTestOpData *pOpData,
    CpaBoolean *pTestPassed)
{
    Cpa32U roundMillerRabin = 0;
    Cpa32U dataLen = 0;
    CpaStatus status = CPA_STATUS_SUCCESS;

    /* ensure LAC is initialized - return error if not */
    LAC_INITIALISED_CHECK();

    /* check for valid callback function pointer */
    LAC_CHECK_NULL_PARAM(pCb);
    /* check for null Operational Data parameters */
    LAC_CHECK_NULL_PARAM(pOpData);
    /* Check for bad pointer */
    LAC_CHECK_NULL_PARAM(pTestPassed);

    /* check for null Prime candidate parameter */
    LAC_CHECK_FLAT_BUFFER(&pOpData->primeCandidate);
  
    /* for the better readability assign the number of rounds for
    *  Miller-Rabin test and Prime Candidate data length */
    roundMillerRabin = pOpData->numMillerRabinRounds;
    dataLen = pOpData->primeCandidate.dataLenInBytes;

    /* Check that the Prime Candidate is within size lilmits, 
    *  not-even (LSB is set), not-null and has MSB set */
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->primeCandidate,
        CHECK_LESS_EQUALS,
        LAC_MAX_OP_SIZE_IN_BITS,
        LAC_CHECK_MSB_YES, LAC_CHECK_LSB_YES);

    /* It is an error if no test is booked */
    if (!((pOpData->performGcdTest) ||
        (pOpData->performFermatTest) ||
        (roundMillerRabin != 0) ||
        (pOpData->performLucasTest)))
        {
            LAC_INVALID_PARAM_LOG("No prime test was selected");
            status = CPA_STATUS_INVALID_PARAM;
        }

    /* Check that, if test has been booked, the Miller-Rabin parameters are set
    *  correctly */
    if ((roundMillerRabin > 0) && (CPA_STATUS_SUCCESS == status))
    {
        /* Number of Miller-Rabin rounds must not exceed maximum allowed! */
        if (roundMillerRabin <= LAC_PRIME_MAX_MILLER_RABIN_ROUNDS)
        {
            /* Check that input buffer size for Miller-Rabin (array of random
            *  numbers) matches that of the Prime Candidate times number of
            *  rounds */
            LAC_CHECK_FLAT_BUFFER_PARAM(
                &pOpData->millerRabinRandomInput,
                CHECK_EQUALS,
                dataLen*roundMillerRabin,
                LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);
        }
        else
        {
            LAC_INVALID_PARAM_LOG("Number of Miller-Rabin rounds too high");
            status = CPA_STATUS_INVALID_PARAM;
        }
    }

    return status;
}
#endif

/**
 ***************************************************************************
 * @ingroup Lac_Prime
 *      Prime Test synchronous function
 ***************************************************************************/

STATIC CpaStatus
LacPrime_TestSyn(const CpaInstanceHandle instanceHandle,
                const CpaCyPrimeTestCbFunc pCb,
                void *pCallbackTag,
                CpaCyPrimeTestOpData *pOpData,
                CpaBoolean *pTestPassed)
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
        status = cpaCyPrimeTest(instanceHandle, 
                LacSync_GenVerifyCb, pSyncCallbackData, 
                pOpData, pTestPassed);
    }
    else
    {
        if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.prime.istat)
        {
            LAC_PRIME_STAT_INC(numPrimeTestRequestErrors, instanceHandle);
        }
        return status;
    }
    if (CPA_STATUS_SUCCESS == status)
    {
        CpaStatus wCbStatus = CPA_STATUS_FAIL;
        wCbStatus = LacSync_WaitForCallback(pSyncCallbackData, 
                        LAC_PKE_SYNC_CALLBACK_TIMEOUT, 
                        &status, pTestPassed);
        if (CPA_STATUS_SUCCESS != wCbStatus)
        {
            /*
             * Inc stats only if the wait for callback failed.
             */
            if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.prime.istat)
            {
                LAC_PRIME_STAT_INC(numPrimeTestCompletedErrors,
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
 * @ingroup Lac_Prime
 *      Prime Test API function
 ******************************************************************************/

CpaStatus
cpaCyPrimeTest(
    const CpaInstanceHandle instanceHandle,
    const CpaCyPrimeTestCbFunc pCb,
    void *pCallbackTag,
    const CpaCyPrimeTestOpData *pOpData,
    CpaBoolean *pTestPassed)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    lac_prime_test_t testId = LAC_PRIME_FERMAT;

    Cpa32U roundMillerRabin = 0;
    Cpa32U functionalityId = 0;
    Cpa32U numRounds = 0;
    Cpa32U indexMillerRabin = 0;
    Cpa32U round = 0;
    Cpa32U dataLen = 0;

    icp_qat_fw_mmp_input_param_t inArgList = { .flat_array = {0} };
    icp_qat_fw_mmp_output_param_t outArgList= { .flat_array = {0} };

    /* Array of random numbers for MIller-Rabin test is stored in the block of
    *  memory. For further processing, one flat buffer has to be formed for each
    *  random number. 
    *  Define array of flat buffers to hold input random numbers
    *  for Miller-Rabin test. */
    CpaFlatBuffer *pBuffInputMillerRabin = NULL;

    /* Data that will be passed back in call back function - opaque data */
    lac_pke_op_cb_data_t primeTestData = {0};

    /* For a single request, or the first request in a chain of requests, the
    *  requestHandle value must be zero (i.e. LAC_PKE_INVALID_HANDLE). The
    *  non-zero value means that the new request is chained to the 
    *  request (chain) already associated with the handle. */
    
    lac_pke_request_handle_t requestHandle = LAC_PKE_INVALID_HANDLE;

    /* Check if the API has been called in sync mode */
    if (NULL == pCb)
    {
        return LacPrime_TestSyn(instanceHandle, pCb,
                   pCallbackTag, LAC_CONST_PTR_CAST(pOpData), 
                               pTestPassed);
    }

#ifdef ICP_PARAM_CHECK
    /* check for valid acceleration handle */
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);

    /* check that the input parameters are valid */
    status = LacPrime_ParameterCheck(pCb, LAC_CONST_PTR_CAST(pOpData),
                                    pTestPassed);
#endif

    if (CPA_STATUS_SUCCESS == status)
    {
        /* for the better readability assign the number of rounds for
        *  Miller-Rabin test and Prime Candidate data length */

        roundMillerRabin = pOpData->numMillerRabinRounds;
        dataLen = pOpData->primeCandidate.dataLenInBytes;
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        /* Allocating the memory for the array of flat buffers in which the
        *  first element is reserved to hold QAT output and the following
        *  elements to hold input random numbers for Miller-Rabin test. */

        pBuffInputMillerRabin = 
            (CpaFlatBuffer*)Lac_MemPoolEntryAlloc(lac_prime_pool);
        if (NULL == pBuffInputMillerRabin)
        {
            LAC_LOG_ERROR("Cannot get mem pool entry");
            status = CPA_STATUS_RESOURCE;
        }

    }

    if ((CPA_STATUS_SUCCESS == status) && (roundMillerRabin > 0))
    {
        /* pointer to the array of random numbers for Miller-Rabin test. Used to
        *  calculate the addresses of the array members */
        Cpa8U *pCurrentAddress = pOpData->millerRabinRandomInput.pData;
    
        /* Link the random number from Miller-Rabin array to the designated
        * member of the array of flat buffers. */

        for (indexMillerRabin = MILLER_RABIN_START_IDX; 
            indexMillerRabin < roundMillerRabin; 
            indexMillerRabin++)
        {            
            /* Members of the array of flat buffers that will hold random
            *  number for M-R test. In fact, each buffer's pData pointer is
            *  assigned the address of the coresponding random number in the 
            *  user given block of memory. The data in block is 64 bit alligned
            *  with leading zeros as required */
           
            pBuffInputMillerRabin[indexMillerRabin].pData = 
                pCurrentAddress;
            pBuffInputMillerRabin[indexMillerRabin].dataLenInBytes =
                dataLen;
            pCurrentAddress += dataLen;
        }
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        /* preserve user parameters and the OutputBuffer for when our Call Back
        *  function kicks in, after sending the head request */

        primeTestData.pClientCb = pCb;
        primeTestData.pCallbackTag = pCallbackTag;
        primeTestData.pClientOpData = pOpData;
        primeTestData.pOpaqueData = pBuffInputMillerRabin;

    }

    /* Create the PKE request and chain as necessary for multiple tests in
    *  order of the increasing complexity: Fermat->Miller-Rabin->Lucas.
    *  Send the request when finished. */

    for (testId = LAC_PRIME_TEST_START_DELIMITER + 1;
        (testId < LAC_PRIME_TEST_END_DELIMITER) && 
        (CPA_STATUS_SUCCESS == status);
        testId++)
    {
        /* assign the number of rounds per test */
        if (testId == LAC_PRIME_GCD)
        {
            numRounds = (pOpData->performGcdTest ? 1 : 0);
        }
        if (testId == LAC_PRIME_FERMAT)
        {
            numRounds = (pOpData->performFermatTest ? 1 : 0);
        }
        if (testId == LAC_PRIME_MILLER_RABIN)
        {
            numRounds = roundMillerRabin;
        }
        if (testId == LAC_PRIME_LUCAS)
        {
            numRounds = (pOpData->performLucasTest ? 1 : 0);
        }

        /* get functionality ID for undergoing test */
        if (numRounds > 0)
        {
            functionalityId = LacPrime_GetFuncID(testId,
                dataLen*LAC_NUM_BITS_IN_BYTE);
            if (LAC_PKE_INVALID_FUNC_ID == functionalityId)
            {
                status =  CPA_STATUS_FAIL;
            }
        }

        /* populate parameters, create request and
        *  update stats for each round */
        
        for (round = 1; (round <= numRounds) && (CPA_STATUS_SUCCESS == status);
            round++)
        {
            LAC_OS_BZERO(&inArgList, sizeof(icp_qat_fw_mmp_input_param_t));
            LAC_OS_BZERO(&outArgList, sizeof(icp_qat_fw_mmp_output_param_t));

            /* populate input/output parameters first */
            LacPrime_PopulateParam(testId, &inArgList, &outArgList, 
                LAC_CONST_PTR_CAST(pOpData),
                (LAC_PRIME_MILLER_RABIN == testId) ?
                &pBuffInputMillerRabin[round-1] : NULL,
                (LAC_PRIME_GCD == testId) ? &lacPrimePsp1 : NULL);

            /* Create PKE request */
            status = LacPke_CreateRequest(&requestHandle, functionalityId,
                NULL, NULL,
                &inArgList, &outArgList,
                LacPrime_TestCallback, &primeTestData);
        }
    }
       
    /* now, after the requests has been created,
    *  send the head request to the QAT */
    if (CPA_STATUS_SUCCESS == status)
    {
        /* send request chain */
        status = LacPke_SendRequest(&requestHandle, instanceHandle);
    }
    /* update stats. In case of failure free the memory */
    if (CPA_STATUS_SUCCESS == status)
    {
        if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.prime.istat)
        {
            /* increment stats:
             *  Total number of prime number test requested operations */
            LAC_PRIME_STAT_INC(numPrimeTestRequests, instanceHandle);
        }
    }
    else
    {
        if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.prime.istat)
        {
            /* on failure increment stats:
             * Total number of prime number test errors recorded */
            LAC_PRIME_STAT_INC(numPrimeTestRequestErrors, instanceHandle);
        }
        /* if allocated, clean the memory on failure */
        
        /* free data from the Input Buffer */
        if (NULL != pBuffInputMillerRabin)
        {
            /* free the array of the input flat buffers */
            Lac_MemPoolEntryFree(pBuffInputMillerRabin);
        }
    }
    return status;
}


/**
 *******************************************************************************
 * @ingroup Lac_Prime
 *      Prime Statistics Query API function
 ******************************************************************************/

CpaStatus
cpaCyPrimeQueryStats(
    CpaInstanceHandle instanceHandle,
    CpaCyPrimeStats *pPrimeStats)
{
    /* ensure LAC is initialized - return error if not */
    LAC_INITIALISED_CHECK();

    if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.prime.istat)
    {
        /* check for valid acceleration handle */
        LAC_CHECK_INSTANCE_HANDLE(instanceHandle);

        /* check for null parameters */
        LAC_CHECK_NULL_PARAM(pPrimeStats);

        /* get stats into user supplied stats structure */
        LAC_PRIME_STATS_GET(*pPrimeStats);
    }
    else
    {
        return CPA_STATUS_RESOURCE;
    }

    return CPA_STATUS_SUCCESS;
}

/**
 *******************************************************************************
 * @ingroup Lac_Prime
 *      Prime Initialization function
 ******************************************************************************/
CpaStatus
LacPrime_Init(Cpa64U numAsymConcurrentReq)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    /* initialize stats to zero */
    LAC_PRIME_STATS_INIT();

    if (NULL == lacPrimePsp1.pData)
    {
        /* allocate lacPrimePsp1 flat buffer data */
        if (CPA_STATUS_SUCCESS != LAC_OS_CAMALLOC(
                &lacPrimePsp1.pData, sizeof(lacPrimePsp1Data), 
                LAC_64BYTE_ALIGNMENT))
        {
            status = CPA_STATUS_FAIL;
        }
    }
    if (CPA_STATUS_SUCCESS == status)
    {
        /* initialize lacPrimePsp1 flat buffer */
        lacPrimePsp1.dataLenInBytes = sizeof(lacPrimePsp1Data);
        memcpy(lacPrimePsp1.pData, lacPrimePsp1Data, sizeof(lacPrimePsp1Data));
    }
    else
    {
        /* on failure free any allocated memory */

        /* free lacPrimePsp1 */
        lacPrimePsp1.dataLenInBytes = 0;
        LAC_OS_CAFREE(lacPrimePsp1.pData,  lacPrimePsp1.dataLenInBytes);
        lacPrimePsp1.pData = NULL;
    }
  
    if (CPA_STATUS_SUCCESS == status)
    {
        status = Lac_MemPoolCreate(&lac_prime_pool, "PRIME",
            numAsymConcurrentReq, sizeof(CpaFlatBuffer) * (MAX_MR + 1),
            LAC_64BYTE_ALIGNMENT, CPA_FALSE);
    }

    return status;
}

/**
 *******************************************************************************
 * @ingroup Lac_Prime
 *      Prime Shutdown function
 ******************************************************************************/
CpaStatus
LacPrime_Shutdown(void)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    
    /* free lacPrimePsp1 */
    lacPrimePsp1.dataLenInBytes = 0;
    
    LAC_OS_CAFREE(lacPrimePsp1.pData,  lacPrimePsp1.dataLenInBytes);
    
    lacPrimePsp1.pData = NULL;

    if (CPA_STATUS_SUCCESS == status)
    {
        Lac_MemPoolDestroy(lac_prime_pool);
    }    
    return status;
}

/**
 *******************************************************************************
 * @ingroup Lac_Prime
 *      Prime Stats Show function
 ******************************************************************************/
void
LacPrime_StatsShow(void)
{
    CpaCyPrimeStats primeStats = {0};
    CpaInstanceHandle instanceHandle = CPA_INSTANCE_HANDLE_SINGLE;

    /* retrieve the stats */
    (void)cpaCyPrimeQueryStats(instanceHandle, &primeStats);

    /* log the stats to the standard output */

    /* Engine Info */
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
        IX_OSAL_LOG_DEV_STDOUT,
        SEPARATOR           
        BORDER " PRIME Stats                                " BORDER "\n"
        SEPARATOR,
        0, 0, 0, 0, 0, 0);

    /* Parameter generation requests - PRIME stats */
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
        IX_OSAL_LOG_DEV_STDOUT,
        BORDER " PRIME successfull requests:     %10u " BORDER "\n"
        BORDER " PRIME failed requests:          %10u " BORDER "\n"
        BORDER " PRIME successfully completed:   %10u " BORDER "\n"
        BORDER " PRIME failed completion:        %10u " BORDER "\n"
        BORDER " PRIME completed - not a prime:  %10u " BORDER "\n"
        SEPARATOR,
        primeStats.numPrimeTestRequests,
        primeStats.numPrimeTestRequestErrors,
        primeStats.numPrimeTestCompleted,
        primeStats.numPrimeTestCompletedErrors,
        primeStats.numPrimeTestFailures,
        0);

}
