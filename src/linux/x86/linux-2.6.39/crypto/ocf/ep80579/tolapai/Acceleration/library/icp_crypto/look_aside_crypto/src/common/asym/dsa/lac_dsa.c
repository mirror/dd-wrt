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
 * @file lac_dsa.c 
 *
 * @ingroup Lac_Dsa
 *
 * This file contains the implementation of DSA functions
 *
 ***************************************************************************/

/*
****************************************************************************
* Include public/global header files
****************************************************************************
*/

/* Include API files */
#include "cpa.h"
#include "cpa_cy_common.h"
#include "cpa_cy_dsa.h"

/* Include Osal files */
#include "IxOsal.h"

/* Include QAT files */
#include "icp_qat_fw_mmp.h"
#include "icp_qat_fw_mmp_ids.h"

/* Include LAC files */
#include "lac_common.h"
#include "lac_dsa.h"
#include "lac_hooks.h"
#include "lac_log.h"
#include "lac_mem.h"
#include "lac_pke_qat_comms.h"
#include "lac_pke_utils.h"

#include "lac_module.h"
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

/**< number of DSA stats */
#define LAC_DSA_NUM_STATS (sizeof(CpaCyDsaStats) / sizeof(Cpa32U))

/**< array of atomics for DSA stats */
STATIC IxOsalAtomic lacDsaStats[LAC_DSA_NUM_STATS];

/**< macro to initialize all DSA stats (stored in internal array of atomics) */
#define LAC_DSA_STATS_INIT()                                                \
do {                                                                        \
    Cpa32U i;                                                               \
                                                                            \
    for (i = 0; i < LAC_DSA_NUM_STATS; i++)                                 \
    {                                                                       \
        ixOsalAtomicSet(0, &lacDsaStats[i]);                                \
    }                                                                       \
} while (0)    

/* macro to increment a DSA stat (derives offset into array of atomics) */
#define LAC_DSA_STAT_INC(statistic, instanceHandle)                           \
do {                                                                          \
    LAC_ASSERT((CPA_INSTANCE_HANDLE_SINGLE == instanceHandle), "Bad Handle"); \
                                                                              \
    ixOsalAtomicInc(                                                          \
        &lacDsaStats[                                                         \
            offsetof(CpaCyDsaStats, statistic) / sizeof(Cpa32U)]);            \
} while (0)

/**< macro to get all DSA stats (from internal array of atomics) */
#define LAC_DSA_STATS_GET(dsaStats)                                         \
do {                                                                        \
    Cpa32U i;                                                               \
                                                                            \
    for (i = 0; i < LAC_DSA_NUM_STATS; i++)                                 \
    {                                                                       \
        ((Cpa32U *)&(dsaStats))[i] = ixOsalAtomicGet(&lacDsaStats[i]);      \
    }                                                                       \
} while (0)    


/**< define sizes of LAC DSA input/output parameters */
#define LAC_DSA_PX_SIZE_IN_BYTES    128 /**< 1024-bit */
#define LAC_DSA_P_SIZE_IN_BYTES     128 /**< 1024-bit */
#define LAC_DSA_Q_SIZE_IN_BYTES      20 /**<  160-bit */
#define LAC_DSA_H_SIZE_IN_BYTES     128 /**< 1024-bit */
#define LAC_DSA_G_SIZE_IN_BYTES     128 /**< 1024-bit */
#define LAC_DSA_X_SIZE_IN_BYTES      20 /**<  160-bit */
#define LAC_DSA_Y_SIZE_IN_BYTES     128 /**< 1024-bit */
#define LAC_DSA_K_SIZE_IN_BYTES      20 /**<  160-bit */
#define LAC_DSA_M_SIZE_IN_BYTES      20 /**<  160-bit */
#define LAC_DSA_R_SIZE_IN_BYTES      20 /**<  160-bit */
#define LAC_DSA_S_SIZE_IN_BYTES      20 /**<  160-bit */
#define LAC_DSA_V_SIZE_IN_BYTES      20 /**<  160-bit */

/**< Product of small primes #1 = 3 * 5 * 7 * 11 * ... * 739 (1024-bit) */
STATIC const Cpa8U lacDsaPsp1Data[] = {
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

/**< Flat buffer for product of small primes #1.  Note pData can't be assigned
   to the lacDsaPsp1Data structure above, as the pData buffer must be allocated
   in shared memory and be properly aligned - instead it is allocated and
   populated during run-time Initialization. */
STATIC CpaFlatBuffer lacDsaPsp1 = { .pData = NULL, .dataLenInBytes = 0 };

/**< Product of small primes #2 = 743 * ... * 1459 (1024-bit) */
STATIC const Cpa8U lacDsaPsp2Data[] = {
    0x02, 0x46, 0xba, 0x2a, 0x39, 0x79, 0x70, 0xbc,
    0xbd, 0x00, 0x77, 0xa6, 0x76, 0x72, 0x73, 0x87,
    0xae, 0x6b, 0x70, 0x9d, 0x09, 0x4e, 0xe5, 0xa8,
    0xdf, 0xae, 0x2c, 0x79, 0xd8, 0xcc, 0xb3, 0xef,
    0x42, 0x87, 0x33, 0x29, 0x9c, 0x3b, 0x66, 0x4b,
    0xe3, 0x5b, 0xfc, 0xe5, 0x85, 0xd2, 0x39, 0xa4,
    0xa1, 0xe3, 0x07, 0xf8, 0xe5, 0xbd, 0x94, 0x28,
    0x4e, 0xa5, 0xa9, 0x57, 0xa4, 0x92, 0xc1, 0x97,
    0x73, 0xcd, 0x8e, 0xbe, 0xcf, 0xc4, 0x98, 0xb0,
    0x7f, 0x69, 0x64, 0x52, 0xfa, 0x99, 0xf8, 0xd7,
    0xa8, 0xad, 0x05, 0xd8, 0x94, 0x09, 0x56, 0xf7,
    0x6e, 0x71, 0xb7, 0xc6, 0x27, 0xc3, 0x35, 0xaa,
    0x03, 0xe0, 0x00, 0x57, 0xce, 0x5e, 0x49, 0xe4,
    0x9e, 0xb2, 0x5a, 0xf8, 0xa7, 0x1b, 0xa6, 0xd5,
    0xab, 0x65, 0x79, 0xe9, 0x2d, 0x34, 0xf4, 0x8e,
    0x81, 0xf9, 0x31, 0xb2, 0x1e, 0xe7, 0x08, 0x09
};

/**< Flat buffer for product of small primes #2.  Note pData can't be assigned
   to the lacDsaPsp2Data structure above, as the pData buffer must be allocated
   in shared memory and be properly aligned - instead it is allocated and
   populated during run-time Initialization. */
STATIC CpaFlatBuffer lacDsaPsp2 = { .pData = NULL, .dataLenInBytes = 0 };

/*
****************************************************************************
* Define static function definitions
****************************************************************************
*/

/*
****************************************************************************
* Global Variables
****************************************************************************
*/

/*
****************************************************************************
* Define public/global function definitions
****************************************************************************
*/

/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA P Parameter Generation internal callback
 ***************************************************************************/
STATIC
void
LacDsaPParamGenCallback(
    CpaStatus status,
    CpaBoolean protocolStatus,
    CpaInstanceHandle instanceHandle,
    lac_pke_op_cb_data_t *pCbData)
{
    CpaCyDsaGenCbFunc pCb = NULL;
    void *pCallbackTag = NULL;
    CpaCyDsaPParamGenOpData *pOpData = NULL;
    CpaFlatBuffer *pP = NULL;

    /* extract info from callback data structure */
    LAC_ASSERT_NOT_NULL(pCbData);
    pCb = (CpaCyDsaGenCbFunc) LAC_CONST_PTR_CAST(pCbData->pClientCb);
    pCallbackTag = pCbData->pCallbackTag;
    pOpData = (CpaCyDsaPParamGenOpData *)
                  LAC_CONST_PTR_CAST(pCbData->pClientOpData);
    pP = pCbData->pOutputData1;

    LAC_ASSERT_NOT_NULL(pCb);
    LAC_ASSERT_NOT_NULL(pOpData);
    LAC_ASSERT_NOT_NULL(pP);

    /* increment stats */
    if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.dsa.istat) 
    {
        if (CPA_STATUS_SUCCESS == status)
        {
            LAC_DSA_STAT_INC(numDsaPParamGenCompleted, instanceHandle);
        }
        else
        {
            LAC_DSA_STAT_INC(numDsaPParamGenCompletedErrors, instanceHandle);
        }
    }
    /* invoke the user callback */
    pCb(pCallbackTag, status, pOpData, protocolStatus, pP);
}

#ifdef ICP_PARAM_CHECK
/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA P Parameter Generation parameter check
 ***************************************************************************/
STATIC
CpaStatus
LacDsaPParamGenParamCheck(
    CpaCyDsaGenCbFunc pCb,
    const CpaCyDsaPParamGenOpData *pOpData,
    CpaBoolean *pProtocolStatus,
    CpaFlatBuffer *pP)
{
    /* ensure LAC is initialized - return error if not */
    LAC_INITIALISED_CHECK();

    /* check for valid callback function pointer */
    LAC_CHECK_NULL_PARAM(pProtocolStatus);
    LAC_CHECK_NULL_PARAM(pP);
    LAC_ENSURE_NOT_NULL(pCb);

    /* check parameters for null, incorrect size, or MSB/LSB not set */
    LAC_CHECK_NULL_PARAM(pOpData);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->X, CHECK_EQUALS, LAC_DSA_PX_SIZE_IN_BYTES,
        LAC_CHECK_MSB_YES, LAC_CHECK_LSB_NO);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->Q, CHECK_EQUALS, LAC_DSA_Q_SIZE_IN_BYTES,
        LAC_CHECK_MSB_YES, LAC_CHECK_LSB_YES);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        pP, CHECK_EQUALS, LAC_DSA_P_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);

    return CPA_STATUS_SUCCESS;
}
#endif

/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA P Parameter Generation synchronous function
 ***************************************************************************/
STATIC CpaStatus
LacDsaPParamGenSyn(const CpaInstanceHandle instanceHandle,
        const CpaCyDsaGenCbFunc pCb,
        void *pCallbackTag,
        const CpaCyDsaPParamGenOpData *pOpData,
        CpaBoolean *pProtocolStatus,
        CpaFlatBuffer *pP)
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
        status = cpaCyDsaGenPParam(instanceHandle, 
                    LacSync_GenFlatBufVerifyCb, pSyncCallbackData, 
                    pOpData, pProtocolStatus, pP);
    }
    else
    {
        LAC_DSA_STAT_INC(numDsaPParamGenRequestErrors, instanceHandle);
        return status;
    }
    if (CPA_STATUS_SUCCESS == status)
    {
        CpaStatus wCbStatus = CPA_STATUS_FAIL;
        wCbStatus = LacSync_WaitForCallback(pSyncCallbackData, 
                        LAC_PKE_SYNC_CALLBACK_TIMEOUT, 
                        &status, pProtocolStatus);
        if (CPA_STATUS_SUCCESS != wCbStatus)
        {
            /*
             * Inc stats only if the wait for callback failed.
             */

            if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.dsa.istat) 
            {
                LAC_DSA_STAT_INC(numDsaPParamGenCompletedErrors,
                        instanceHandle);
            }
            status = wCbStatus;
        }
    }
    LacSync_DestroySyncCookie(&pSyncCallbackData);
    return status;
}

/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA P Parameter Generation API function
 ***************************************************************************/
CpaStatus
cpaCyDsaGenPParam(const CpaInstanceHandle instanceHandle,
        const CpaCyDsaGenCbFunc pCb,
        void *pCallbackTag,
        const CpaCyDsaPParamGenOpData *pOpData,
        CpaBoolean *pProtocolStatus,
        CpaFlatBuffer *pP)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    icp_qat_fw_mmp_input_param_t in = { .flat_array = {0} };
    icp_qat_fw_mmp_output_param_t out = { .flat_array = {0} };
    lac_pke_op_cb_data_t cbData = {0};

    /* Check if the API has been called in sync mode */
    if (NULL == pCb)
    {
        return LacDsaPParamGenSyn(instanceHandle, pCb,
                    pCallbackTag, pOpData, pProtocolStatus, pP);
    }

#ifdef ICP_PARAM_CHECK
    /* check for valid acceleration handle - can't update stats otherwise */
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);

    /* check remaining parameters */
    status = LacDsaPParamGenParamCheck(pCb, pOpData, pProtocolStatus, pP);
#endif

    if (CPA_STATUS_SUCCESS == status)
    {
        /* populate input parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_gen_p_1024_160.x, &pOpData->X);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_gen_p_1024_160.q, &pOpData->Q);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_gen_p_1024_160.psp1, &lacDsaPsp1);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_gen_p_1024_160.psp2, &lacDsaPsp2);

        /* populate output parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            out.mmp_dsa_gen_p_1024_160.p, pP);

        /* populate callback data */
        cbData.pClientCb = pCb;
        cbData.pCallbackTag = pCallbackTag;
        cbData.pClientOpData = pOpData;
        cbData.pOpaqueData = NULL;
        cbData.pOutputData1 = pP;

        /* send a PKE request to the QAT */
        status = LacPke_SendSingleRequest(
            PKE_DSA_GEN_P_1024_160, NULL, NULL, &in, &out,
            LacDsaPParamGenCallback, &cbData, instanceHandle);
    }

    /* increment stats */
    if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.dsa.istat) 
    {
        if (CPA_STATUS_SUCCESS == status)
        {
            LAC_DSA_STAT_INC(numDsaPParamGenRequests, instanceHandle);
        }
        else
        {
            LAC_DSA_STAT_INC(numDsaPParamGenRequestErrors, instanceHandle);
        }
    }

    return status;
}

/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA G Parameter Generation internal callback
 ***************************************************************************/
STATIC
void
LacDsaGParamGenCallback(
    CpaStatus status,
    CpaBoolean protocolStatus,
    CpaInstanceHandle instanceHandle,
    lac_pke_op_cb_data_t *pCbData)
{
    CpaCyDsaGenCbFunc pCb = NULL;
    void *pCallbackTag = NULL;
    CpaCyDsaGParamGenOpData *pOpData = NULL;
    CpaFlatBuffer *pG = NULL;

    /* extract info from callback data structure */
    LAC_ASSERT_NOT_NULL(pCbData);
    pCb = (CpaCyDsaGenCbFunc) LAC_CONST_PTR_CAST(pCbData->pClientCb);
    pCallbackTag = pCbData->pCallbackTag;
    pOpData = (CpaCyDsaGParamGenOpData *)
                  LAC_CONST_PTR_CAST(pCbData->pClientOpData);
    pG = pCbData->pOutputData1;

    LAC_ASSERT_NOT_NULL(pCb);
    LAC_ASSERT_NOT_NULL(pOpData);
    LAC_ASSERT_NOT_NULL(pG);

    /* increment stats */
    if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.dsa.istat) 
    {
        if (CPA_STATUS_SUCCESS == status)
        {
            LAC_DSA_STAT_INC(numDsaGParamGenCompleted, instanceHandle);
        }
        else
        {
            LAC_DSA_STAT_INC(numDsaGParamGenCompletedErrors, instanceHandle);
        }
    }

    /* invoke the user callback */
    pCb(pCallbackTag, status, pOpData, protocolStatus, pG);
}

#ifdef ICP_PARAM_CHECK
/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA G Parameter Generation parameter check
 ***************************************************************************/
STATIC
CpaStatus
LacDsaGParamGenParamCheck(
    const CpaCyDsaGenCbFunc pCb,
    const CpaCyDsaGParamGenOpData *pOpData,
    CpaBoolean *pProtocolStatus,
    CpaFlatBuffer *pG)
{
    /* ensure LAC is initialized - return error if not */
    LAC_INITIALISED_CHECK();

    /* check for valid callback function pointer */
    LAC_CHECK_NULL_PARAM(pCb);

    /* check for valid output pointers */
    LAC_CHECK_NULL_PARAM(pProtocolStatus);
    LAC_CHECK_NULL_PARAM(pG);

    /* check parameters for null, incorrect size, or MSB/LSB not set */
    LAC_CHECK_NULL_PARAM(pOpData);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->P, CHECK_EQUALS, LAC_DSA_P_SIZE_IN_BYTES,
        LAC_CHECK_MSB_YES, LAC_CHECK_LSB_YES);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->Q, CHECK_EQUALS, LAC_DSA_Q_SIZE_IN_BYTES,
        LAC_CHECK_MSB_YES, LAC_CHECK_LSB_YES);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->H, CHECK_EQUALS, LAC_DSA_H_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        pG, CHECK_EQUALS, LAC_DSA_G_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);

    return CPA_STATUS_SUCCESS;
}
#endif

/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA G Parameter Generation synchronous function
 ***************************************************************************/
STATIC CpaStatus
LacDsaGParamGenSyn(const CpaInstanceHandle instanceHandle,
        const CpaCyDsaGenCbFunc pCb,
        void *pCallbackTag,
        const CpaCyDsaGParamGenOpData *pOpData,
        CpaBoolean *pProtocolStatus,
        CpaFlatBuffer *pG)
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
        status = cpaCyDsaGenGParam(instanceHandle, 
                     LacSync_GenFlatBufVerifyCb, pSyncCallbackData,
                     pOpData, pProtocolStatus, pG);
    }
    else
    {
        if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.dsa.istat) 
        {
            LAC_DSA_STAT_INC(numDsaGParamGenRequestErrors, instanceHandle);
        }
        return status;
    }
    if (CPA_STATUS_SUCCESS == status)
    {
        CpaStatus wCbStatus = CPA_STATUS_FAIL;
        wCbStatus = LacSync_WaitForCallback(pSyncCallbackData, 
                        LAC_PKE_SYNC_CALLBACK_TIMEOUT, 
                        &status, pProtocolStatus);
        if (CPA_STATUS_SUCCESS != wCbStatus)
        {
            /*
             * Inc stats only if the wait for callback failed.
             */
            if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.dsa.istat) 
            {
                LAC_DSA_STAT_INC(numDsaGParamGenCompletedErrors,
                        instanceHandle);
            }
            status = wCbStatus;
        }
    }
    LacSync_DestroySyncCookie(&pSyncCallbackData);
    return status;
}


/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA G Parameter Generation API function
 ***************************************************************************/
CpaStatus
cpaCyDsaGenGParam(const CpaInstanceHandle instanceHandle,
        const CpaCyDsaGenCbFunc pCb,
        void *pCallbackTag,
        const CpaCyDsaGParamGenOpData *pOpData,
        CpaBoolean *pProtocolStatus,
        CpaFlatBuffer *pG)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    icp_qat_fw_mmp_input_param_t in = { .flat_array = {0} };
    icp_qat_fw_mmp_output_param_t out = { .flat_array = {0} };
    lac_pke_op_cb_data_t cbData = {0};

    /* Check if the API has been called in sync mode */
    if (NULL == pCb)
    {
        return LacDsaGParamGenSyn(instanceHandle, pCb,
                    pCallbackTag, pOpData, pProtocolStatus, pG);
    }

#ifdef ICP_PARAM_CHECK
    /* check for valid acceleration handle - can't update stats otherwise */
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);

    /* check remaining parameters */
    status = LacDsaGParamGenParamCheck(pCb, pOpData, pProtocolStatus, pG);
#endif

    if (CPA_STATUS_SUCCESS == status)
    {
        /* populate input parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_gen_g_1024_160.p, &pOpData->P);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_gen_g_1024_160.q, &pOpData->Q);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_gen_g_1024_160.h, &pOpData->H);

        /* populate output parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            out.mmp_dsa_gen_g_1024_160.g, pG);

        /* populate callback data */
        cbData.pClientCb = pCb;
        cbData.pCallbackTag = pCallbackTag;
        cbData.pClientOpData = pOpData;
        cbData.pOpaqueData = NULL;
        cbData.pOutputData1 = pG;

        /* send a PKE request to the QAT */
        status = LacPke_SendSingleRequest(
            PKE_DSA_GEN_G_1024_160, NULL, NULL, &in, &out,
            LacDsaGParamGenCallback, &cbData, instanceHandle);
    }

    /* increment stats */
    if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.dsa.istat) 
    {
        if (CPA_STATUS_SUCCESS == status)
        {
            LAC_DSA_STAT_INC(numDsaGParamGenRequests, instanceHandle);
        }
        else
        {
            LAC_DSA_STAT_INC(numDsaGParamGenRequestErrors, instanceHandle);
        }
    }
    return status;
}


/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA Y Parameter Generation internal callback
 ***************************************************************************/
STATIC
void
LacDsaYParamGenCallback(
    CpaStatus status,
    CpaBoolean protocolStatus,
    CpaInstanceHandle instanceHandle,
    lac_pke_op_cb_data_t *pCbData)
{
    CpaCyDsaGenCbFunc pCb = NULL;
    void *pCallbackTag = NULL;
    CpaCyDsaYParamGenOpData *pOpData = NULL;
    CpaFlatBuffer *pY = NULL;

    /* extract info from callback data structure */
    LAC_ASSERT_NOT_NULL(pCbData);
    pCb = (CpaCyDsaGenCbFunc) LAC_CONST_PTR_CAST(pCbData->pClientCb);
    pCallbackTag = pCbData->pCallbackTag;
    pOpData = (CpaCyDsaYParamGenOpData *)
                  LAC_CONST_PTR_CAST(pCbData->pClientOpData);
    pY = pCbData->pOutputData1;

    LAC_ASSERT_NOT_NULL(pCb);
    LAC_ASSERT_NOT_NULL(pOpData);
    LAC_ASSERT_NOT_NULL(pY);

    /* increment stats */
    if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.dsa.istat) 
    {
        if (CPA_STATUS_SUCCESS == status)
        {
            LAC_DSA_STAT_INC(numDsaYParamGenCompleted, instanceHandle);
        }
        else
        {
            LAC_DSA_STAT_INC(numDsaYParamGenCompletedErrors, instanceHandle);
        }
    }

    /* invoke the user callback */
    pCb(pCallbackTag, status, pOpData, protocolStatus, pY);
}

#ifdef ICP_PARAM_CHECK
/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA Y Parameter Generation parameter check
 ***************************************************************************/
STATIC
CpaStatus
LacDsaYParamGenParamCheck(
    const CpaCyDsaGenCbFunc pCb,
    const CpaCyDsaYParamGenOpData *pOpData,
    CpaBoolean *pProtocolStatus,
    CpaFlatBuffer *pY)
{
    /* ensure LAC is initialized - return error if not */
    LAC_INITIALISED_CHECK();

    /* check for valid callback function pointer */
    LAC_CHECK_NULL_PARAM(pCb);

    /* check for valid output pointers */
    LAC_CHECK_NULL_PARAM(pProtocolStatus);
    LAC_CHECK_NULL_PARAM(pY);

    /* check parameters for null, incorrect size, or MSB/LSB not set */
    LAC_CHECK_NULL_PARAM(pOpData);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->P, CHECK_EQUALS, LAC_DSA_P_SIZE_IN_BYTES,
        LAC_CHECK_MSB_YES, LAC_CHECK_LSB_YES);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->G, CHECK_EQUALS, LAC_DSA_G_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->X, CHECK_EQUALS, LAC_DSA_X_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        pY, CHECK_EQUALS, LAC_DSA_Y_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);

    return CPA_STATUS_SUCCESS;
}
#endif

/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA Y Parameter Generation synchronous function
 ***************************************************************************/
STATIC CpaStatus
LacDsaYParamGenSyn(const CpaInstanceHandle instanceHandle,
        const CpaCyDsaGenCbFunc pCb,
        void *pCallbackTag,
        const CpaCyDsaYParamGenOpData *pOpData,
        CpaBoolean *pProtocolStatus,
        CpaFlatBuffer *pY)
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
        status = cpaCyDsaGenYParam(instanceHandle, 
                    LacSync_GenFlatBufVerifyCb, pSyncCallbackData, 
                    pOpData, pProtocolStatus, pY);
    }
    else
    {
        LAC_DSA_STAT_INC(numDsaYParamGenRequestErrors, instanceHandle);
        return status;
    }
    if (CPA_STATUS_SUCCESS == status)
    {
        CpaStatus wCbStatus = CPA_STATUS_FAIL;
        wCbStatus = LacSync_WaitForCallback(pSyncCallbackData, 
                        LAC_PKE_SYNC_CALLBACK_TIMEOUT, 
                        &status, pProtocolStatus);
        if (CPA_STATUS_SUCCESS != wCbStatus)
        {
            /*
             * Inc stats only if the wait for callback failed.
             */
            LAC_DSA_STAT_INC(numDsaYParamGenCompletedErrors,
                    instanceHandle);
            status = wCbStatus;
        }
    }
    LacSync_DestroySyncCookie(&pSyncCallbackData);
    return status;
}

/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA Y Parameter Generation API function
 ***************************************************************************/
CpaStatus
cpaCyDsaGenYParam(const CpaInstanceHandle instanceHandle,
        const CpaCyDsaGenCbFunc pCb,
        void *pCallbackTag,
        const CpaCyDsaYParamGenOpData *pOpData,
        CpaBoolean *pProtocolStatus,
        CpaFlatBuffer *pY)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    icp_qat_fw_mmp_input_param_t in = { .flat_array = {0} };
    icp_qat_fw_mmp_output_param_t out = { .flat_array = {0} };
    lac_pke_op_cb_data_t cbData = {0};

    /* Check if the API has been called in sync mode */
    if (NULL == pCb)
    {
        return LacDsaYParamGenSyn(instanceHandle, pCb,
                    pCallbackTag, pOpData, pProtocolStatus, pY);
    }

#ifdef ICP_PARAM_CHECK
    /* check for valid acceleration handle - can't update stats otherwise */
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);

    /* check remaining parameters */
    status = LacDsaYParamGenParamCheck(pCb, pOpData, pProtocolStatus, pY);
#endif

    if (CPA_STATUS_SUCCESS == status)
    {
        /* populate input parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_gen_y_1024_160.p, &pOpData->P);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_gen_y_1024_160.g, &pOpData->G);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_gen_y_1024_160.x, &pOpData->X);

        /* populate output parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            out.mmp_dsa_gen_y_1024_160.y, pY);

        /* populate callback data */
        cbData.pClientCb = pCb;
        cbData.pCallbackTag = pCallbackTag;
        cbData.pClientOpData = pOpData;
        cbData.pOpaqueData = NULL;
        cbData.pOutputData1 = pY;

        /* send a PKE request to the QAT */
        status = LacPke_SendSingleRequest(
            PKE_DSA_GEN_Y_1024_160, NULL, NULL, &in, &out,
            LacDsaYParamGenCallback, &cbData, instanceHandle);
    }

    /* increment stats */
    if (CPA_STATUS_SUCCESS == status)
    {
        LAC_DSA_STAT_INC(numDsaYParamGenRequests, instanceHandle);
    }
    else
    {
        LAC_DSA_STAT_INC(numDsaYParamGenRequestErrors, instanceHandle);
    }

    return status;
}

/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA R Sign internal callback
 ***************************************************************************/
STATIC
void
LacDsaRSignCallback(
    CpaStatus status,
    CpaBoolean protocolStatus,
    CpaInstanceHandle instanceHandle,
    lac_pke_op_cb_data_t *pCbData)
{
    CpaCyDsaGenCbFunc pCb = NULL;
    void *pCallbackTag = NULL;
    CpaCyDsaRSignOpData *pOpData = NULL;
    CpaFlatBuffer *pR = NULL;

    /* extract info from callback data structure */
    LAC_ASSERT_NOT_NULL(pCbData);
    pCb = (CpaCyDsaGenCbFunc) LAC_CONST_PTR_CAST(pCbData->pClientCb);
    pCallbackTag = pCbData->pCallbackTag;
    pOpData = (CpaCyDsaRSignOpData *)
                  LAC_CONST_PTR_CAST(pCbData->pClientOpData);
    pR = pCbData->pOutputData1;

    LAC_ASSERT_NOT_NULL(pCb);
    LAC_ASSERT_NOT_NULL(pOpData);
    LAC_ASSERT_NOT_NULL(pR);

    /* increment stats */
    if (CPA_STATUS_SUCCESS == status)
    {
        LAC_DSA_STAT_INC(numDsaRSignCompleted, instanceHandle);
    }
    else
    {
        LAC_DSA_STAT_INC(numDsaRSignCompletedErrors, instanceHandle);
    }

    /* invoke the user callback */
    pCb(pCallbackTag, status, pOpData, protocolStatus, pR);
}

#ifdef ICP_PARAM_CHECK
/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA R Sign parameter check
 ***************************************************************************/
STATIC
CpaStatus
LacDsaRSignParamCheck(
    const CpaCyDsaGenCbFunc pCb,
    const CpaCyDsaRSignOpData *pOpData,
    CpaBoolean *pProtocolStatus,
    CpaFlatBuffer *pR)
{
    /* ensure LAC is initialized - return error if not */
    LAC_INITIALISED_CHECK();

    /* check for valid callback function pointer */
    LAC_CHECK_NULL_PARAM(pCb);

    /* check for output pointers */
    LAC_CHECK_NULL_PARAM(pProtocolStatus);
    LAC_CHECK_NULL_PARAM(pR);

    /* check parameters for null, incorrect size, or MSB/LSB not set */
    LAC_CHECK_NULL_PARAM(pOpData);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->K, CHECK_EQUALS, LAC_DSA_K_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->P, CHECK_EQUALS, LAC_DSA_P_SIZE_IN_BYTES,
        LAC_CHECK_MSB_YES, LAC_CHECK_LSB_YES);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->Q, CHECK_EQUALS, LAC_DSA_Q_SIZE_IN_BYTES,
        LAC_CHECK_MSB_YES, LAC_CHECK_LSB_YES);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->G, CHECK_EQUALS, LAC_DSA_G_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        pR, CHECK_EQUALS, LAC_DSA_R_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);

    return CPA_STATUS_SUCCESS;
}
#endif

/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA R Sign synchronous function
 ***************************************************************************/
STATIC CpaStatus
LacDsaRSignSyn(const CpaInstanceHandle instanceHandle,
        const CpaCyDsaGenCbFunc pCb,
        void *pCallbackTag,
        const CpaCyDsaRSignOpData *pOpData,
        CpaBoolean *pProtocolStatus,
        CpaFlatBuffer *pR)
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
        status = cpaCyDsaSignR(instanceHandle, 
                    LacSync_GenFlatBufVerifyCb, pSyncCallbackData,
                    pOpData, pProtocolStatus, pR);
    }
    else
    {
        LAC_DSA_STAT_INC(numDsaRSignRequestErrors, instanceHandle);
        return status;
    }
    if (CPA_STATUS_SUCCESS == status)
    {
        CpaStatus wCbStatus = CPA_STATUS_FAIL;
        wCbStatus = LacSync_WaitForCallback(pSyncCallbackData, 
                        LAC_PKE_SYNC_CALLBACK_TIMEOUT, 
                        &status, pProtocolStatus);
        if (CPA_STATUS_SUCCESS != wCbStatus)
        {
            /*
             * Inc stats only if the wait for callback failed.
             */
            LAC_DSA_STAT_INC(numDsaRSignCompletedErrors,
                    instanceHandle);
            status = wCbStatus;
        }
    }
    LacSync_DestroySyncCookie(&pSyncCallbackData);
    return status;
}

/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA R Sign API function
 ***************************************************************************/
CpaStatus
cpaCyDsaSignR(const CpaInstanceHandle instanceHandle,
        const CpaCyDsaGenCbFunc pCb,
        void *pCallbackTag,
        const CpaCyDsaRSignOpData *pOpData,
        CpaBoolean *pProtocolStatus,
        CpaFlatBuffer *pR)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    icp_qat_fw_mmp_input_param_t in = { .flat_array = {0} };
    icp_qat_fw_mmp_output_param_t out = { .flat_array = {0} };
    lac_pke_op_cb_data_t cbData = {0};

    /* Check if the API has been called in sync mode */
    if (NULL == pCb)
    {
        return LacDsaRSignSyn(instanceHandle, pCb,
                pCallbackTag, pOpData, pProtocolStatus, pR);
    }

#ifdef ICP_PARAM_CHECK
    /* check for valid acceleration handle - can't update stats otherwise */
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);

    /* check remaining parameters */
    status = LacDsaRSignParamCheck(pCb, pOpData, pProtocolStatus, pR);
#endif

    if (CPA_STATUS_SUCCESS == status)
    {
        /* populate input parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_sign_r_1024_160.k, &pOpData->K);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_sign_r_1024_160.p, &pOpData->P);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_sign_r_1024_160.q, &pOpData->Q);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_sign_r_1024_160.g, &pOpData->G);

        /* populate output parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            out.mmp_dsa_sign_r_1024_160.r, pR);

        /* populate callback data */
        cbData.pClientCb = pCb;
        cbData.pCallbackTag = pCallbackTag;
        cbData.pClientOpData = pOpData;
        cbData.pOpaqueData = NULL;
        cbData.pOutputData1 = pR;

        /* send a PKE request to the QAT */
        status = LacPke_SendSingleRequest(
            PKE_DSA_SIGN_R_1024_160, NULL, NULL, &in, &out,
            LacDsaRSignCallback, &cbData, instanceHandle);
    }

    /* increment stats */
    if (CPA_STATUS_SUCCESS == status)
    {
        LAC_DSA_STAT_INC(numDsaRSignRequests, instanceHandle);
    }
    else
    {
        LAC_DSA_STAT_INC(numDsaRSignRequestErrors, instanceHandle);
    }

    return status;
}


/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA S Sign internal callback
 ***************************************************************************/
STATIC
void
LacDsaSSignCallback(
    CpaStatus status,
    CpaBoolean protocolStatus,
    CpaInstanceHandle instanceHandle,
    lac_pke_op_cb_data_t *pCbData)
{
    CpaCyDsaGenCbFunc pCb = NULL;
    void *pCallbackTag = NULL;
    CpaCyDsaSSignOpData *pOpData = NULL;
    CpaFlatBuffer *pS = NULL;

    /* extract info from callback data structure */
    LAC_ASSERT_NOT_NULL(pCbData);
    pCb = (CpaCyDsaGenCbFunc) LAC_CONST_PTR_CAST(pCbData->pClientCb);
    pCallbackTag = pCbData->pCallbackTag;
    pOpData = (CpaCyDsaSSignOpData *)LAC_CONST_PTR_CAST(pCbData->pClientOpData);
    pS = pCbData->pOutputData1;

    LAC_ASSERT_NOT_NULL(pCb);
    LAC_ASSERT_NOT_NULL(pOpData);
    LAC_ASSERT_NOT_NULL(pS);

    /* increment stats */
    if (CPA_STATUS_SUCCESS == status)
    {
        LAC_DSA_STAT_INC(numDsaSSignCompleted, instanceHandle);
    }
    else
    {
        LAC_DSA_STAT_INC(numDsaSSignCompletedErrors, instanceHandle);
    }

    /* invoke the user callback */
    pCb(pCallbackTag, status, pOpData, protocolStatus, pS);
}

#ifdef ICP_PARAM_CHECK
/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA S Sign parameter check
 ***************************************************************************/
STATIC
CpaStatus
LacDsaSSignParamCheck(
        const CpaCyDsaGenCbFunc pCb,
        const CpaCyDsaSSignOpData *pOpData,
        CpaBoolean *pProtocolStatus,
        CpaFlatBuffer *pS)
{
    /* ensure LAC is initialized - return error if not */
    LAC_INITIALISED_CHECK();

    /* check for valid callback function pointer */
    LAC_CHECK_NULL_PARAM(pCb);

    /* check for valid out buffer and data pointers */
    LAC_CHECK_NULL_PARAM(pS);
    LAC_CHECK_NULL_PARAM(pProtocolStatus);

    /* check parameters for null, incorrect size, or MSB/LSB not set */
    LAC_CHECK_NULL_PARAM(pOpData);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->M, CHECK_EQUALS, LAC_DSA_M_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->K, CHECK_EQUALS, LAC_DSA_K_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->Q, CHECK_EQUALS, LAC_DSA_Q_SIZE_IN_BYTES,
        LAC_CHECK_MSB_YES, LAC_CHECK_LSB_YES);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->R, CHECK_EQUALS, LAC_DSA_R_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->X, CHECK_EQUALS, LAC_DSA_X_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        pS, CHECK_EQUALS, LAC_DSA_S_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);

    return CPA_STATUS_SUCCESS;
}
#endif

/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA S Sign synchronous function
 ***************************************************************************/
STATIC CpaStatus
LacDsaSignSSyn(const CpaInstanceHandle instanceHandle,
        const CpaCyDsaGenCbFunc pCb,
        void *pCallbackTag,
        const CpaCyDsaSSignOpData *pOpData,
        CpaBoolean *pProtocolStatus,
        CpaFlatBuffer *pS)
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
        status = cpaCyDsaSignS(instanceHandle, 
                    LacSync_GenFlatBufVerifyCb, pSyncCallbackData, 
                    pOpData, pProtocolStatus, pS);
    }
    else
    {
        LAC_DSA_STAT_INC(numDsaSSignRequestErrors, instanceHandle);
        return status;
    }
    if (CPA_STATUS_SUCCESS == status)
    {
        CpaStatus wCbStatus = CPA_STATUS_FAIL;
        wCbStatus = LacSync_WaitForCallback(pSyncCallbackData, 
                        LAC_PKE_SYNC_CALLBACK_TIMEOUT, 
                        &status, pProtocolStatus);
        if (CPA_STATUS_SUCCESS != wCbStatus)
        {
            /*
             * Inc stats only if the wait for callback failed.
             */
            LAC_DSA_STAT_INC(numDsaSSignCompletedErrors,
                    instanceHandle);
            status = wCbStatus;
        }
    }
    LacSync_DestroySyncCookie(&pSyncCallbackData);
    return status;
}

/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA S Sign API function
 ***************************************************************************/
CpaStatus
cpaCyDsaSignS(const CpaInstanceHandle instanceHandle,
        const CpaCyDsaGenCbFunc pCb,
        void *pCallbackTag,
        const CpaCyDsaSSignOpData *pOpData,
        CpaBoolean *pProtocolStatus,
        CpaFlatBuffer *pS)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    icp_qat_fw_mmp_input_param_t in = { .flat_array = {0} };
    icp_qat_fw_mmp_output_param_t out = { .flat_array = {0} };
    lac_pke_op_cb_data_t cbData = {0};

    /* Check if the API has been called in sync mode */
    if (NULL == pCb)
    {
        return LacDsaSignSSyn(instanceHandle, pCb,
                pCallbackTag, pOpData, pProtocolStatus, pS);
    }

#ifdef ICP_PARAM_CHECK
    /* check for valid acceleration handle - can't update stats otherwise */
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);

    /* check remaining parameters */
    status = LacDsaSSignParamCheck(pCb, pOpData, pProtocolStatus, pS);
#endif

    if (CPA_STATUS_SUCCESS == status)
    {
        /* populate input parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_sign_s_1024_160.m, &pOpData->M);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_sign_s_1024_160.k, &pOpData->K);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_sign_s_1024_160.q, &pOpData->Q);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_sign_s_1024_160.r, &pOpData->R);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_sign_s_1024_160.x, &pOpData->X);

        /* populate output parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            out.mmp_dsa_sign_s_1024_160.s, pS);

        /* populate callback data */
        cbData.pClientCb = pCb;
        cbData.pCallbackTag = pCallbackTag;
        cbData.pClientOpData = pOpData;
        cbData.pOpaqueData = NULL;
        cbData.pOutputData1 = pS;

        /* send a PKE request to the QAT */
        status = LacPke_SendSingleRequest(
            PKE_DSA_SIGN_S_1024_160, NULL, NULL, &in, &out,
            LacDsaSSignCallback, &cbData, instanceHandle);
    }

    /* increment stats */
    if (CPA_STATUS_SUCCESS == status)
    {
        LAC_DSA_STAT_INC(numDsaSSignRequests, instanceHandle);
    }
    else
    {
        LAC_DSA_STAT_INC(numDsaSSignRequestErrors, instanceHandle);
    }

    return status;
}


/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA RS Sign internal callback
 ***************************************************************************/
STATIC
void
LacDsaRSSignCallback(
    CpaStatus status,
    CpaBoolean protocolStatus,
    CpaInstanceHandle instanceHandle,
    lac_pke_op_cb_data_t *pCbData)
{
    CpaCyDsaRSSignCbFunc pCb = NULL;
    void *pCallbackTag = NULL;
    CpaCyDsaRSSignOpData *pOpData = NULL;
    CpaFlatBuffer *pR = NULL;
    CpaFlatBuffer *pS = NULL;

    /* extract info from callback data structure */
    LAC_ASSERT_NOT_NULL(pCbData);
    pCb = (CpaCyDsaRSSignCbFunc) LAC_CONST_PTR_CAST(pCbData->pClientCb);
    pCallbackTag = pCbData->pCallbackTag;
    pOpData = (CpaCyDsaRSSignOpData *)
                  LAC_CONST_PTR_CAST(pCbData->pClientOpData);
    pR = pCbData->pOutputData1;
    pS = pCbData->pOutputData2;

    LAC_ASSERT_NOT_NULL(pCb);
    LAC_ASSERT_NOT_NULL(pOpData);
    LAC_ASSERT_NOT_NULL(pR);
    LAC_ASSERT_NOT_NULL(pS);

    /* increment stats */
    if (CPA_STATUS_SUCCESS == status)
    {
        LAC_DSA_STAT_INC(numDsaRSSignCompleted, instanceHandle);
    }
    else
    {
        LAC_DSA_STAT_INC(numDsaRSSignCompletedErrors, instanceHandle);
    }

    /* invoke the user callback */
    pCb(pCallbackTag, status, pOpData, protocolStatus, pR, pS);
}

#ifdef ICP_PARAM_CHECK
/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA RS Sign parameter check
 ***************************************************************************/
STATIC
CpaStatus
LacDsaRSSignParamCheck(
    const CpaCyDsaRSSignCbFunc pCb,
    const CpaCyDsaRSSignOpData *pOpData,
    CpaBoolean *pProtocolStatus,
    CpaFlatBuffer *pR,
    CpaFlatBuffer *pS)
{
    /* ensure LAC is initialized - return error if not */
    LAC_INITIALISED_CHECK();

    /* check for valid callback function pointer */
    LAC_CHECK_NULL_PARAM(pCb);

    /* check for valid out buffer pointers */
    LAC_CHECK_NULL_PARAM(pProtocolStatus);
    LAC_CHECK_NULL_PARAM(pR);
    LAC_CHECK_NULL_PARAM(pS);

    /* check parameters for null, incorrect size, or MSB/LSB not set */
    LAC_CHECK_NULL_PARAM(pOpData);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->M, CHECK_EQUALS, LAC_DSA_M_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->K, CHECK_EQUALS, LAC_DSA_K_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->P, CHECK_EQUALS, LAC_DSA_P_SIZE_IN_BYTES,
        LAC_CHECK_MSB_YES, LAC_CHECK_LSB_YES);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->Q, CHECK_EQUALS, LAC_DSA_Q_SIZE_IN_BYTES,
        LAC_CHECK_MSB_YES, LAC_CHECK_LSB_YES);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->G, CHECK_EQUALS, LAC_DSA_G_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->X, CHECK_EQUALS, LAC_DSA_X_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        pR, CHECK_EQUALS, LAC_DSA_R_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        pS, CHECK_EQUALS, LAC_DSA_S_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);

    return CPA_STATUS_SUCCESS;
}
#endif

/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA RS Sign synchronous function
 ***************************************************************************/
STATIC CpaStatus
LacDsaRSSignSyn(const CpaInstanceHandle instanceHandle,
        const CpaCyDsaRSSignCbFunc pCb,
        void *pCallbackTag,
        const CpaCyDsaRSSignOpData *pOpData,
        CpaBoolean *pProtocolStatus,
        CpaFlatBuffer *pR,
        CpaFlatBuffer *pS)
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
        status = cpaCyDsaSignRS(instanceHandle, 
                    LacSync_GenDualFlatBufVerifyCb, 
                    pSyncCallbackData, pOpData, pProtocolStatus,
                    pR, pS);
    }
    else
    {
        LAC_DSA_STAT_INC(numDsaRSSignRequestErrors, instanceHandle);
        return status;
    }
    if (CPA_STATUS_SUCCESS == status)
    {
        CpaStatus wCbStatus = CPA_STATUS_FAIL;
        wCbStatus = LacSync_WaitForCallback(pSyncCallbackData, 
                        LAC_PKE_SYNC_CALLBACK_TIMEOUT, 
                        &status, pProtocolStatus);
        if (CPA_STATUS_SUCCESS != wCbStatus)
        {
            /*
             * Inc stats only if the wait for callback failed.
             */
            LAC_DSA_STAT_INC(numDsaRSSignCompletedErrors,
                    instanceHandle);
            status = wCbStatus;
        }
    }
    LacSync_DestroySyncCookie(&pSyncCallbackData);
    return status;
}


/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA RS Sign API function
 ***************************************************************************/
CpaStatus
cpaCyDsaSignRS(const CpaInstanceHandle instanceHandle,
        const CpaCyDsaRSSignCbFunc pCb,
        void *pCallbackTag,
        const CpaCyDsaRSSignOpData *pOpData,
        CpaBoolean *pProtocolStatus,
        CpaFlatBuffer *pR,
        CpaFlatBuffer *pS)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    icp_qat_fw_mmp_input_param_t in = { .flat_array = {0} };
    icp_qat_fw_mmp_output_param_t out = { .flat_array = {0} };
    lac_pke_op_cb_data_t cbData = {0};

    /* Check if the API has been called in sync mode */
    if (NULL == pCb)
    {
        return LacDsaRSSignSyn(instanceHandle, pCb, pCallbackTag,
                pOpData, pProtocolStatus, pR, pS);
    }
    
#ifdef ICP_PARAM_CHECK
    /* check for valid acceleration handle - can't update stats otherwise */
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);

    /* check remaining parameters */
    status = LacDsaRSSignParamCheck(pCb, pOpData, pProtocolStatus, pR, pS);
#endif

    if (CPA_STATUS_SUCCESS == status)
    {
        /* populate input parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_sign_r_s_1024_160.m, &pOpData->M);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_sign_r_s_1024_160.k, &pOpData->K);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_sign_r_s_1024_160.p, &pOpData->P);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_sign_r_s_1024_160.q, &pOpData->Q);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_sign_r_s_1024_160.g, &pOpData->G);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_sign_r_s_1024_160.x, &pOpData->X);

        /* populate output parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            out.mmp_dsa_sign_r_s_1024_160.r, pR);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            out.mmp_dsa_sign_r_s_1024_160.s, pS);

        /* populate callback data */
        cbData.pClientCb = pCb;
        cbData.pCallbackTag = pCallbackTag;
        cbData.pClientOpData = pOpData;
        cbData.pOpaqueData = NULL;
        cbData.pOutputData1 = pR;
        cbData.pOutputData2 = pS;

        /* send a PKE request to the QAT */
        status = LacPke_SendSingleRequest(
            PKE_DSA_SIGN_R_S_1024_160, NULL, NULL, &in, &out,
            LacDsaRSSignCallback, &cbData, instanceHandle);
    }

    /* increment stats */
    if (CPA_STATUS_SUCCESS == status)
    {
        LAC_DSA_STAT_INC(numDsaRSSignRequests, instanceHandle);
    }
    else
    {
        LAC_DSA_STAT_INC(numDsaRSSignRequestErrors, instanceHandle);
    }

    return status;
}


/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA Verify internal callback
 ***************************************************************************/
STATIC
void
LacDsaVerifyCallback(
    CpaStatus status,
    CpaBoolean pass,
    CpaInstanceHandle instanceHandle,
    lac_pke_op_cb_data_t *pCbData)
{
    CpaCyDsaVerifyCbFunc pCb = NULL;
    void *pCallbackTag = NULL;
    CpaCyDsaVerifyOpData *pOpData = NULL;

    /* extract info from callback data structure */
    LAC_ASSERT_NOT_NULL(pCbData);
    pCb = (CpaCyDsaVerifyCbFunc) LAC_CONST_PTR_CAST(pCbData->pClientCb);
    pCallbackTag = pCbData->pCallbackTag;
    pOpData = (CpaCyDsaVerifyOpData *)
                  LAC_CONST_PTR_CAST(pCbData->pClientOpData);

    LAC_ASSERT_NOT_NULL(pCb);
    LAC_ASSERT_NOT_NULL(pOpData);

    /* increment stats */
    if (CPA_STATUS_SUCCESS == status)
    {
        LAC_DSA_STAT_INC(numDsaVerifyCompleted, instanceHandle);
        if (CPA_FALSE == pass)
        {
            LAC_DSA_STAT_INC(numDsaVerifyFailures, instanceHandle);
        }
    }
    else
    {
        LAC_DSA_STAT_INC(numDsaVerifyCompletedErrors, instanceHandle);
    }

    /* invoke the user callback */
    pCb(pCallbackTag, status, pOpData, pass);
}

#ifdef ICP_PARAM_CHECK
/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA Verify parameter check
 ***************************************************************************/
STATIC
CpaStatus
LacDsaVerifyParamCheck(
    const CpaCyDsaVerifyCbFunc pCb,
    const CpaCyDsaVerifyOpData *pOpData,
    CpaBoolean *pVerifyStatus)
{
    /* ensure LAC is initialized - return error if not */
    LAC_INITIALISED_CHECK();

    /* check for valid callback function pointer */
    LAC_CHECK_NULL_PARAM(pCb);

    /* check for valid output pointer */
    LAC_CHECK_NULL_PARAM(pVerifyStatus);

    /* check parameters for null, incorrect size, or MSB/LSB not set */
    LAC_CHECK_NULL_PARAM(pOpData);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->R, CHECK_EQUALS, LAC_DSA_R_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->S, CHECK_EQUALS, LAC_DSA_S_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->M, CHECK_EQUALS, LAC_DSA_M_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->P, CHECK_EQUALS, LAC_DSA_P_SIZE_IN_BYTES,
        LAC_CHECK_MSB_YES, LAC_CHECK_LSB_YES);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->Q, CHECK_EQUALS, LAC_DSA_Q_SIZE_IN_BYTES,
        LAC_CHECK_MSB_YES, LAC_CHECK_LSB_YES);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->G, CHECK_EQUALS, LAC_DSA_G_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);
    LAC_CHECK_FLAT_BUFFER_PARAM(
        &pOpData->Y, CHECK_EQUALS, LAC_DSA_Y_SIZE_IN_BYTES,
        LAC_CHECK_MSB_NO, LAC_CHECK_LSB_NO);

    return CPA_STATUS_SUCCESS;
}
#endif

/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA Verify synchronous function
 ***************************************************************************/
STATIC CpaStatus
LacDsaVerifySyn(const CpaInstanceHandle instanceHandle,
        const CpaCyDsaVerifyCbFunc pCb,
        void *pCallbackTag,
        const CpaCyDsaVerifyOpData *pOpData,
        CpaBoolean *pVerifyStatus)
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
        status = cpaCyDsaVerify(instanceHandle, 
                    LacSync_GenVerifyCb, pSyncCallbackData, 
                    pOpData, pVerifyStatus);
    }
    else
    {
        LAC_DSA_STAT_INC(numDsaVerifyRequests, instanceHandle);
        return status;
    }
    if (CPA_STATUS_SUCCESS == status)
    {
        CpaStatus wCbStatus = CPA_STATUS_FAIL;
        wCbStatus = LacSync_WaitForCallback(pSyncCallbackData, 
                        LAC_PKE_SYNC_CALLBACK_TIMEOUT, 
                        &status, pVerifyStatus);
        if (CPA_STATUS_SUCCESS != wCbStatus)
        {
            /*
             * Inc stats only if the wait for callback failed.
             */
            LAC_DSA_STAT_INC(numDsaVerifyCompletedErrors,
                    instanceHandle);
            status = wCbStatus;
        }
    }
    LacSync_DestroySyncCookie(&pSyncCallbackData);
    return status;
}

/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA Verify API function
 ***************************************************************************/
CpaStatus
cpaCyDsaVerify(const CpaInstanceHandle instanceHandle,
        const CpaCyDsaVerifyCbFunc pCb,
        void *pCallbackTag,
        const CpaCyDsaVerifyOpData *pOpData,
        CpaBoolean *pVerifyStatus)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    icp_qat_fw_mmp_input_param_t in = { .flat_array = {0} };
    icp_qat_fw_mmp_output_param_t out = { .flat_array = {0} };
    lac_pke_op_cb_data_t cbData = {0};

    /* Check if the API has been called in sync mode */
    if (NULL == pCb)
    {
        return LacDsaVerifySyn(instanceHandle, pCb,
                pCallbackTag, pOpData, pVerifyStatus);
    }

#ifdef ICP_PARAM_CHECK
    /* check for valid acceleration handle - can't update stats otherwise */
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);

    /* check remaining parameters */
    status = LacDsaVerifyParamCheck(pCb, pOpData, pVerifyStatus);
#endif

    /** @mem_usage A 20B (LAC_DSA_V_SIZE_IN_BYTES) buffer is allocated here to
        store the result V of the verification - the client never sees this
        buffer */

    /** @performance It would be better to allocate a pool of flat buffers
        at initialization time, instead of dynamically allocating them (pV)
        for each request. */

    /** @performance It would be better if the QAT took care of performing
       the comparison of the result V to the input R, and just returned a
       status of pass/fail */


    if (CPA_STATUS_SUCCESS == status)
    {
        /* populate input parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_verify_1024_160.r, &pOpData->R);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_verify_1024_160.s, &pOpData->S);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_verify_1024_160.m, &pOpData->M);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_verify_1024_160.p, &pOpData->P);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_verify_1024_160.q, &pOpData->Q);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_verify_1024_160.g, &pOpData->G);
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.mmp_dsa_verify_1024_160.y, &pOpData->Y);

        /* populate callback data */
        cbData.pClientCb = pCb;
        cbData.pCallbackTag = pCallbackTag;
        cbData.pClientOpData = pOpData;
        cbData.pOpaqueData = NULL;

        /* send a PKE request to the QAT */
        status = LacPke_SendSingleRequest(
            PKE_DSA_VERIFY_1024_160, NULL, NULL, &in, &out,
            LacDsaVerifyCallback, &cbData, instanceHandle);
    }

    /* increment stats (may change pending Work Request IXA00191914) */
    if (CPA_STATUS_SUCCESS == status)
    {
        LAC_DSA_STAT_INC(numDsaVerifyRequests, instanceHandle);
    }
    else
    {
        LAC_DSA_STAT_INC(numDsaVerifyRequestErrors, instanceHandle);
    }

    return status;
}


/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA Statistics Query API function
 ***************************************************************************/
CpaStatus
cpaCyDsaQueryStats(
    CpaInstanceHandle instanceHandle,
    CpaCyDsaStats *pDsaStats)
{
    /* ensure LAC is initialized - return error if not */
    LAC_INITIALISED_CHECK();

    /* check for valid acceleration handle */
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);

    /* check for null parameters */
    LAC_CHECK_NULL_PARAM(pDsaStats);

    /* get stats into user supplied stats structure */
    if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.dsa.istat) 
    {
        LAC_DSA_STATS_GET(*pDsaStats);
    }
    else
    {
        return CPA_STATUS_RESOURCE;
    }

    return CPA_STATUS_SUCCESS;
}

/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA Initialization function
 ***************************************************************************/
CpaStatus
LacDsa_Init(void)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    /** @mem_usage two 1024-bit flat buffers are allocated here, coming to a
       total of 256B */

    /* initialize stats to zero */
    LAC_DSA_STATS_INIT();

    if (NULL == lacDsaPsp1.pData)

    {
        /* allocate lacDsaPsp1 flat buffer data */
        if (CPA_STATUS_SUCCESS != LAC_OS_CAMALLOC(
                &lacDsaPsp1.pData, sizeof(lacDsaPsp1Data),
                LAC_64BYTE_ALIGNMENT))
        {
            status = CPA_STATUS_FAIL;
        }
    }

    if (NULL == lacDsaPsp2.pData)
    {
        /* allocate lacDsaPsp2 flat buffer data */
        if (CPA_STATUS_SUCCESS != LAC_OS_CAMALLOC(
                &lacDsaPsp2.pData, sizeof(lacDsaPsp2Data),
                LAC_64BYTE_ALIGNMENT))
        {
            status = CPA_STATUS_FAIL;
        }
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        /* initialize lacDsaPsp1 flat buffer */
        lacDsaPsp1.dataLenInBytes = sizeof(lacDsaPsp1Data);
        memcpy(lacDsaPsp1.pData, lacDsaPsp1Data, sizeof(lacDsaPsp1Data));

        /* initialize lacDsaPsp2 flat buffer */
        lacDsaPsp2.dataLenInBytes = sizeof(lacDsaPsp2Data);
        memcpy(lacDsaPsp2.pData, lacDsaPsp2Data, sizeof(lacDsaPsp2Data));
    }
    else
    {
        /* on failure free any allocated memory */

        /* free lacDsaPsp1 */
        lacDsaPsp1.dataLenInBytes = 0;
        LAC_OS_CAFREE(lacDsaPsp1.pData, sizeof(lacDsaPsp1Data));
        lacDsaPsp1.pData = NULL;

        /* free lacDsaPsp2 */
        lacDsaPsp2.dataLenInBytes = 0;
        LAC_OS_CAFREE(lacDsaPsp2.pData, sizeof(lacDsaPsp2Data));
        lacDsaPsp2.pData = NULL;
    }

    return status;
}

/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA Shutdown function
 ***************************************************************************/
CpaStatus
LacDsa_Shutdown(void)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    /* free lacDsaPsp1 */
    lacDsaPsp1.dataLenInBytes = 0;
    LAC_OS_CAFREE(lacDsaPsp1.pData, sizeof(lacDsaPsp1Data));

    lacDsaPsp1.pData = NULL;

    /* free lacDsaPsp2 */
    lacDsaPsp2.dataLenInBytes = 0;
    LAC_OS_CAFREE(lacDsaPsp2.pData, sizeof(lacDsaPsp2Data));
    
    lacDsaPsp2.pData = NULL;

    return status;
}

/**
 ***************************************************************************
 * @ingroup Lac_Dsa
 *      DSA Stats Show function
 ***************************************************************************/
void
LacDsa_StatsShow(void)
{
#if defined(__linux)
    CpaCyDsaStats dsaStats = {0};
#elif defined(__freebsd)
    CpaCyDsaStats dsaStats;
#endif
    CpaInstanceHandle instanceHandle = CPA_INSTANCE_HANDLE_SINGLE;

#if defined(__freebsd)
    LAC_OS_BZERO((void *) &dsaStats, sizeof(dsaStats));
#endif

    /* retrieve the stats */
    (void)cpaCyDsaQueryStats(instanceHandle, &dsaStats);

    /* log the stats to the standard output */

    /* engine info */
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
        IX_OSAL_LOG_DEV_STDOUT,
        SEPARATOR
        BORDER "  DSA Stats                                 " BORDER "\n"
        SEPARATOR,
        0, 0, 0, 0, 0, 0);

    /* p parameter generation requests */
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
        IX_OSAL_LOG_DEV_STDOUT,
        BORDER " DSA P Param Gen Requests-Succ:  %10u " BORDER "\n"
        BORDER " DSA P Param Gen Requests-Err:   %10u " BORDER "\n"
        BORDER " DSA P Param Gen Completed-Succ: %10u " BORDER "\n"
        BORDER " DSA P Param Gen Completed-Err:  %10u " BORDER "\n"
        SEPARATOR,
        dsaStats.numDsaPParamGenRequests,
        dsaStats.numDsaPParamGenRequestErrors,
        dsaStats.numDsaPParamGenCompleted,
        dsaStats.numDsaPParamGenCompletedErrors,
        0, 0);

    /* g parameter generation requests */
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
        IX_OSAL_LOG_DEV_STDOUT,
        BORDER " DSA G Param Gen Requests-Succ:  %10u " BORDER "\n"
        BORDER " DSA G Param Gen Requests-Err:   %10u " BORDER "\n"
        BORDER " DSA G Param Gen Completed-Succ: %10u " BORDER "\n"
        BORDER " DSA G Param Gen Completed-Err:  %10u " BORDER "\n"
        SEPARATOR,
        dsaStats.numDsaGParamGenRequests,
        dsaStats.numDsaGParamGenRequestErrors,
        dsaStats.numDsaGParamGenCompleted,
        dsaStats.numDsaGParamGenCompletedErrors,
        0, 0);

    /* y parameter generation requests */
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
        IX_OSAL_LOG_DEV_STDOUT,
        BORDER " DSA Y Param Gen Requests-Succ:  %10u " BORDER "\n"
        BORDER " DSA Y Param Gen Requests-Err:   %10u " BORDER "\n"
        BORDER " DSA Y Param Gen Completed-Succ: %10u " BORDER "\n"
        BORDER " DSA Y Param Gen Completed-Err:  %10u " BORDER "\n"
        SEPARATOR,
        dsaStats.numDsaYParamGenRequests,
        dsaStats.numDsaYParamGenRequestErrors,
        dsaStats.numDsaYParamGenCompleted,
        dsaStats.numDsaYParamGenCompletedErrors,
        0, 0);

    /* r sign requests */
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
        IX_OSAL_LOG_DEV_STDOUT,
        BORDER " DSA R Sign Requests-Succ:       %10u " BORDER "\n"
        BORDER " DSA R Sign Request-Err:         %10u " BORDER "\n"
        BORDER " DSA R Sign Completed-Succ:      %10u " BORDER "\n"
        BORDER " DSA R Sign Completed-Err:       %10u " BORDER "\n"
        SEPARATOR,
        dsaStats.numDsaRSignRequests,
        dsaStats.numDsaRSignRequestErrors,
        dsaStats.numDsaRSignCompleted,
        dsaStats.numDsaRSignCompletedErrors,
        0, 0);

    /* s sign requests */
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
        IX_OSAL_LOG_DEV_STDOUT,
        BORDER " DSA S Sign Requests-Succ:       %10u " BORDER "\n"
        BORDER " DSA S Sign Request-Err:         %10u " BORDER "\n"
        BORDER " DSA S Sign Completed-Succ:      %10u " BORDER "\n"
        BORDER " DSA S Sign Completed-Err:       %10u " BORDER "\n"
        SEPARATOR,
        dsaStats.numDsaSSignRequests,
        dsaStats.numDsaSSignRequestErrors,
        dsaStats.numDsaSSignCompleted,
        dsaStats.numDsaSSignCompletedErrors,
        0, 0);

    /* rs sign requests */
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
        IX_OSAL_LOG_DEV_STDOUT,
        BORDER " DSA RS Sign Requests-Succ:      %10u " BORDER "\n"
        BORDER " DSA RS Sign Request-Err:        %10u " BORDER "\n"
        BORDER " DSA RS Sign Completed-Succ:     %10u " BORDER "\n"
        BORDER " DSA RS Sign Completed-Err:      %10u " BORDER "\n"
        SEPARATOR,
        dsaStats.numDsaRSSignRequests,
        dsaStats.numDsaRSSignRequestErrors,
        dsaStats.numDsaRSSignCompleted,
        dsaStats.numDsaRSSignCompletedErrors,
        0, 0);

    /* verify requests */
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
        IX_OSAL_LOG_DEV_STDOUT,
        BORDER " DSA Verify Requests-Succ:       %10u " BORDER "\n"
        BORDER " DSA Verify Request-Err:         %10u " BORDER "\n"
        BORDER " DSA Verify Completed-Succ:      %10u " BORDER "\n"
        BORDER " DSA Verify Completed-Err:       %10u " BORDER "\n"
        BORDER " DSA Verify Completed-Failure:   %10u " BORDER "\n"
        SEPARATOR,
        dsaStats.numDsaVerifyRequests,
        dsaStats.numDsaVerifyRequestErrors,
        dsaStats.numDsaVerifyCompleted,
        dsaStats.numDsaVerifyCompletedErrors,
        dsaStats.numDsaVerifyFailures,
        0);
}
