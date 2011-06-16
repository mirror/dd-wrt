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
 * @file lac_sym_key.c
 *
 * @ingroup LacSymKey
 *
 * This file contains the implementation of all keygen functionality
 *
 *****************************************************************************/

/*
*******************************************************************************
* Include public/global header files
*******************************************************************************
*/
#include "cpa.h"
#include "cpa_cy_key.h"
#include "icp_qat_fw.h"
#include "icp_qat_fw_la.h"
#include "qat_comms.h"
#include "IxOsal.h"

#include "lac_mem.h"
#include "lac_hooks.h"
#include "lac_common.h"
#include "lac_sync.h"
#include "lac_sym.h"
#include "lac_sym_key.h"
#include "lac_sym_qat.h"
#include "lac_sym_qat_hash.h"
#include "lac_sym_qat_key.h"
#include "lac_sym_hash_defs.h"

#include "lac_module.h"

/*
*******************************************************************************
* Define Labels for TLS
*******************************************************************************
*/

STATIC Cpa8U *pSslLabel = NULL;
/**< @ingroup LacSymKey
 * Label for SSL. Size is 136 bytes for 16 iterations, which can theroretically
 *  generate up to 256 bytes of output data. QAT will generate a maximum of 
 * 255 bytes */

#define LAC_SYM_KEY_TLS_MASTER_SECRET_LABEL     ("master secret")
/**< @ingroup LacSymKey
 * Label for TLS Master Secret Key Derivation, as defined in RFC4346 */

#define LAC_SYM_KEY_TLS_KEY_MATERIAL_LABEL      ("key expansion")
/**< @ingroup LacSymKey
 * Label for TLS Key Material Generation, as defined in RFC4346. */

#define LAC_SYM_KEY_TLS_CLIENT_FIN_LABEL        ("client finished")
/**< @ingroup LacSymKey
 * Label for TLS Client finished Message, as defined in RFC4346. */

#define LAC_SYM_KEY_TLS_SERVER_FIN_LABEL        ("server finished")
/**< @ingroup LacSymKey
 * Label for TLS Server finished Message, as defined in RFC4346. */


/**
 ******************************************************************************
 * @ingroup LacSymKey 
 *      TLS label struct
 *
 * @description
 *      This structure is used to hold the various TLS labels. Each field is
 *      on an 8 byte boundary provided the structure itslef is 8 bytes aligned.
 *****************************************************************************/
typedef struct lac_sym_key_tls_labels_s
{
    Cpa8U masterSecret[ICP_QAT_FW_LA_TLS_LABEL_LEN_MAX];
    /**< Master secret label */
    Cpa8U keyMaterial[ICP_QAT_FW_LA_TLS_LABEL_LEN_MAX];
    /**< Key material label */
    Cpa8U clientFinished[ICP_QAT_FW_LA_TLS_LABEL_LEN_MAX];
    /**< client finished label */
    Cpa8U serverFinished[ICP_QAT_FW_LA_TLS_LABEL_LEN_MAX];
    /**< server finished label */
} lac_sym_key_tls_labels_t;

STATIC lac_sym_key_tls_labels_t *pTlsLabel = NULL;
/**< @ingroup LacSymKey
 * Instance of the structure containing all the TLS labels. The memory for it 
 * is dynamically allocated at init time.  */

/*
*******************************************************************************
* Define Constants and Macros for SSL, TLS and MGF
*******************************************************************************
*/

#define LAC_SYM_KEY_NO_HASH_BLK_OFFSET_BYTES                0
/**< Used to indicate there is no hash block offset in the content descriptor 
 */

#define LAC_KEY_NUM_STATS (sizeof(CpaCyKeyGenStats) / sizeof(Cpa32U))
/**< number of Key stats */

STATIC IxOsalAtomic lacKeyStats[LAC_KEY_NUM_STATS];
/**< array of atomics for Key stats */

#define LAC_KEY_STAT_INC(statistic, instanceHandle)                           \
do {                                                                          \
    LAC_ASSERT((CPA_INSTANCE_HANDLE_SINGLE == instanceHandle), "Bad Handle"); \
                                                                              \
    ixOsalAtomicInc(                                                          \
        &lacKeyStats[                                                         \
            offsetof(CpaCyKeyGenStats, statistic) / sizeof(Cpa32U)]);         \
} while (0)
/**< macro to increment a Key stat (derives offset into array of atomics) */


#define LAC_KEY_STATS_GET(keyStats)                                         \
do {                                                                        \
    int i;                                                                  \
                                                                            \
    for (i = 0; i < LAC_KEY_NUM_STATS; i++)                                 \
    {                                                                       \
        ((Cpa32U *)&(keyStats))[i] = ixOsalAtomicGet(&lacKeyStats[i]);      \
    }                                                                       \
} while (0)
/**< macro to get all Key stats (from internal array of atomics) */

/**
 ******************************************************************************
 * @ingroup LacSymKey 
 *      SSL/TLS stat type
 *
 * @description
 *      This enum determines which stat should be incremented
 *****************************************************************************/
typedef enum
{
    LAC_KEY_REQUESTS = 0,
    /**< Key requests sent */
    LAC_KEY_REQUEST_ERRORS,
    /**< Key requests errors */
    LAC_KEY_COMPLETED,
    /**< Key requests which received responses */
    LAC_KEY_COMPLETED_ERRORS
    /**< Key requests which received responses with errors */
}lac_key_stat_type_t;


/*** Local functions prototypes ***/
STATIC void
LacSymKey_MgfHandleResponse(
    icp_qat_fw_la_cmd_id_t lacCmdId,
    void *pOpaqueData,
    icp_qat_fw_comn_flags cmnRespFlags);

STATIC CpaStatus
LacSymKey_MgfSync(const CpaInstanceHandle instanceHandle,
               const CpaCyGenFlatBufCbFunc pKeyGenCb,
               void *pCallbackTag,
               const CpaCyKeyGenMgfOpData *pKeyGenMgfOpData,
               CpaFlatBuffer *pGeneratedMaskBuffer);
 
STATIC void
LacSymKey_SslTlsHandleResponse(icp_qat_fw_la_cmd_id_t lacCmdId,
    void *pOpaqueData,
    icp_qat_fw_comn_flags cmnRespFlags);

STATIC CpaStatus
LacSymKey_SslTlsSync(
    CpaInstanceHandle instanceHandle,
    const CpaCyGenFlatBufCbFunc pKeyGenCb,
    void *pCallbackTag,
    icp_qat_fw_la_cmd_id_t lacCmdId,
    void *pKeyGenSslTlsOpData,
    Cpa8U *pData);


/*** Implementation ***/

/**
*******************************************************************************
 * @ingroup LacSymKey
 *      Perform SSL/TLS key gen operation
 *
 * @description
 *      Perform SSL/TLS key gen operation
 *
 * @param[in] instanceHandle        QAT device handle.
 * @param[in] pKeyGenCb             Pointer to callback function to be invoked
 *                                  when the operation is complete.
 * @param[in] pCallbackTag          Opaque User Data for this specific call.
 * @param[in] lacCmdId              Lac command ID (identify SSL & TLS ops)
 * @param[in] pKeyGenSslTlsOpData   Structure containing all the data needed to
 *                                  perform the SSL/TLS key generation
 *                                  operation.
 * @param[out] pData                pointer to where output result should be
 *                                  written
 *
 * @retval CPA_STATUS_SUCCESS       Function executed successfully.
 * @retval CPA_STATUS_FAIL           Function failed.
 * @retval CPA_STATUS_RETRY          Function should be retried.
 * @retval CPA_STATUS_INVALID_PARAM  Invalid parameter passed in.
 * @retval CPA_STATUS_RESOURCE       Error related to system resources.
 *
 *****************************************************************************/
CpaStatus
LacKeyGenSslTls_GenCommon(
    CpaInstanceHandle instanceHandle,
    const CpaCyGenFlatBufCbFunc pKeyGenCb,
    void *pCallbackTag,
    icp_qat_fw_la_cmd_id_t lacCmdId,
    void *pKeyGenSslTlsOpData,
    Cpa8U *pData);


/**
 ******************************************************************************
 * @ingroup LacSymKey 
 *      Increment stat for TLS or SSL operation
 *
 * @description
 *      This is a generic function to update the stats for either a TLS or SSL
 *      operation. 
 * 
 * @param[in] lacCmdId          Indicate SSL or TLS operations
 * @param[in] statType          Statistics Type
 * @param[in] instanceHandle    Instance Handle
 * 
 * @return None
 *
 *****************************************************************************/
STATIC void
LacKey_StatsInc(
    icp_qat_fw_la_cmd_id_t lacCmdId,
    lac_key_stat_type_t statType,
    CpaInstanceHandle instanceHandle)
{
    if (ICP_QAT_FW_LA_CMD_SSL3_KEY_DERIVE == lacCmdId)
    {
        switch (statType)
        {
            case LAC_KEY_REQUESTS:
                LAC_KEY_STAT_INC(numSslKeyGenRequests, instanceHandle);
                break;
            case LAC_KEY_REQUEST_ERRORS:
                LAC_KEY_STAT_INC(numSslKeyGenRequestErrors, instanceHandle);
                break;
            case LAC_KEY_COMPLETED:
                LAC_KEY_STAT_INC(numSslKeyGenCompleted, instanceHandle);
                break;
            case LAC_KEY_COMPLETED_ERRORS:
                LAC_KEY_STAT_INC(numSslKeyGenCompletedErrors, instanceHandle);
                break;
            default:
                LAC_ENSURE(CPA_FALSE, "Invalid statistics type");
                break;
        }
    }
    else    /* TLS */
    {
        switch (statType)
        {
            case LAC_KEY_REQUESTS:
                LAC_KEY_STAT_INC(numTlsKeyGenRequests, instanceHandle);
                break;
            case LAC_KEY_REQUEST_ERRORS:
                LAC_KEY_STAT_INC(numTlsKeyGenRequestErrors, instanceHandle);
                break;
            case LAC_KEY_COMPLETED:
                LAC_KEY_STAT_INC(numTlsKeyGenCompleted, instanceHandle);
                break;
            case LAC_KEY_COMPLETED_ERRORS:
                LAC_KEY_STAT_INC(numTlsKeyGenCompletedErrors, instanceHandle);
                break;
            default:
                LAC_ENSURE(CPA_FALSE, "Invalid statistics type");
                break;
        }
    }
}


void
LacKeygen_StatsShow(void)
{
    CpaCyKeyGenStats keyStats = {0};

    LAC_KEY_STATS_GET(keyStats);

    ixOsalLog (IX_OSAL_LOG_LVL_USER,
               IX_OSAL_LOG_DEV_STDOUT,
               SEPARATOR
               BORDER "                  Key Stats:                " BORDER "\n"
               SEPARATOR,
               0, 0, 0, 0, 0, 0);

    ixOsalLog (IX_OSAL_LOG_LVL_USER,
               IX_OSAL_LOG_DEV_STDOUT,
               BORDER " SSL Key Requests:               %10u " BORDER "\n"
               BORDER " SSL Key Request Errors:         %10u " BORDER "\n"
               BORDER " SSL Key Completed               %10u " BORDER "\n"
               BORDER " SSL Key Complete Errors:        %10u " BORDER "\n"
               SEPARATOR,
               keyStats.numSslKeyGenRequests,
               keyStats.numSslKeyGenRequestErrors,
               keyStats.numSslKeyGenCompleted,
               keyStats.numSslKeyGenCompletedErrors,
               0, 0);

    ixOsalLog (IX_OSAL_LOG_LVL_USER,
               IX_OSAL_LOG_DEV_STDOUT,
               BORDER " TLS Key Requests:               %10u " BORDER "\n"
               BORDER " TLS Key Request Errors:         %10u " BORDER "\n"
               BORDER " TLS Key Completed               %10u " BORDER "\n"
               BORDER " TLS Key Complete Errors:        %10u " BORDER "\n"
               SEPARATOR,
               keyStats.numTlsKeyGenRequests,
               keyStats.numTlsKeyGenRequestErrors,
               keyStats.numTlsKeyGenCompleted,
               keyStats.numTlsKeyGenCompletedErrors,
               0, 0);

    ixOsalLog (IX_OSAL_LOG_LVL_USER,
               IX_OSAL_LOG_DEV_STDOUT,
               BORDER " MGF Key Requests:               %10u " BORDER "\n"
               BORDER " MGF Key Request Errors:         %10u " BORDER "\n"
               BORDER " MGF Key Completed               %10u " BORDER "\n"
               BORDER " MGF Key Complete Errors:        %10u " BORDER "\n"
               SEPARATOR,
               keyStats.numMgfKeyGenRequests,
               keyStats.numMgfKeyGenRequestErrors,
               keyStats.numMgfKeyGenCompleted,
               keyStats.numMgfKeyGenCompletedErrors,
               0, 0);

}

/** @ingroup LacSymKey */
CpaStatus
cpaCyKeyGenQueryStats(CpaInstanceHandle instanceHandle,
                        CpaCyKeyGenStats *pSymKeyStats)
{
#ifdef ICP_PARAM_CHECK
    LAC_INITIALISED_CHECK();
#endif //ICP_PARAM_CHECK

    if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.random.istat)
    {
#ifdef ICP_PARAM_CHECK
        LAC_CHECK_NULL_PARAM(pSymKeyStats);
        LAC_CHECK_INSTANCE_HANDLE(instanceHandle);
#endif //ICP_PARAM_CHECK

        LAC_KEY_STATS_GET(*pSymKeyStats);
    } 
    else
    {
        return CPA_STATUS_RESOURCE;
    }

    return CPA_STATUS_SUCCESS;
}


/**
 ******************************************************************************
 * @ingroup LacSymKey
 *      Key Generation MGF response handler
 *
 * @description
 *      Handles Key Generation MGF response messages from the QAT.
 *
 * @param[in] lacCmdId       Command id of the original request
 * @param[in] pOpaqueData    Pointer to opaque data that was in request
 * @param[in] cmnRespFlags   Indicates whether request succeeded
 *
 * @return void
 *
 *****************************************************************************/
STATIC void
LacSymKey_MgfHandleResponse(
    icp_qat_fw_la_cmd_id_t lacCmdId,
    void *pOpaqueData,
    icp_qat_fw_comn_flags cmnRespFlags)
{
    CpaCyKeyGenMgfOpData *pMgfOpData = NULL;
    lac_sym_key_cookie_t *pCookie = NULL;
    CpaCyGenFlatBufCbFunc pKeyGenMgfCb = NULL;
    void *pCallbackTag = NULL;
    CpaFlatBuffer *pGeneratedKeyBuffer= NULL;
    CpaStatus status = CPA_STATUS_SUCCESS;
    CpaBoolean respStatusOk = (ICP_QAT_FW_COMN_STATUS_FLAG_OK ==
                               ICP_QAT_FW_COMN_STATUS_GET(cmnRespFlags))
                              ? CPA_TRUE : CPA_FALSE;

    pCookie = (lac_sym_key_cookie_t *)pOpaqueData;

    if (CPA_TRUE == respStatusOk)
    {
        status = CPA_STATUS_SUCCESS;
        LAC_KEY_STAT_INC(numMgfKeyGenCompleted, pCookie->instanceHandle);
    }
    else
    {
        status = CPA_STATUS_FAIL;
        LAC_KEY_STAT_INC(numMgfKeyGenCompletedErrors, pCookie->instanceHandle);
    }

    pKeyGenMgfCb =
        (CpaCyGenFlatBufCbFunc)(pCookie->pKeyGenCb);

    LAC_ASSERT_NOT_NULL(pKeyGenMgfCb);

    pMgfOpData = pCookie->pKeyGenOpData;
    pCallbackTag = pCookie->pCallbackTag;
    pGeneratedKeyBuffer = (CpaFlatBuffer *)pCookie->pKeyGenOpData;

    Lac_MemPoolEntryFree(pCookie);

    (*pKeyGenMgfCb)(pCallbackTag, status, pMgfOpData, pGeneratedKeyBuffer);

}


STATIC CpaStatus
LacSymKey_MgfSync(const CpaInstanceHandle instanceHandle,
               const CpaCyGenFlatBufCbFunc pKeyGenCb,
               void *pCallbackTag,
               const CpaCyKeyGenMgfOpData *pKeyGenMgfOpData,
               CpaFlatBuffer *pGeneratedMaskBuffer)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    lac_sync_op_data_t *pSyncCallbackData = NULL;

    status = LacSync_CreateSyncCookie(&pSyncCallbackData);

    if (CPA_STATUS_SUCCESS == status)
    {
        status = cpaCyKeyGenMgf(instanceHandle, LacSync_GenFlatBufCb,
                                pSyncCallbackData, pKeyGenMgfOpData,
                                pGeneratedMaskBuffer);
    }
    else
    {
        /* Failure allocating sync cookie */
        LAC_KEY_STAT_INC(numMgfKeyGenRequestErrors, instanceHandle);
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        CpaStatus syncStatus = CPA_STATUS_SUCCESS;

        syncStatus = LacSync_WaitForCallback(pSyncCallbackData,
                                             LAC_SYM_SYNC_CALLBACK_TIMEOUT,
                                             &status,
                                             NULL);

        /* If callback doesn't come back */
        if (CPA_STATUS_SUCCESS != syncStatus)
        {
            LAC_KEY_STAT_INC(numMgfKeyGenCompletedErrors, instanceHandle);
            LAC_LOG_ERROR("Callback timed out");
            status = syncStatus;
        }
    }

    LacSync_DestroySyncCookie(&pSyncCallbackData);

    return status;
}


/**
* cpaCyKeyGenMgf
*/
CpaStatus
cpaCyKeyGenMgf(const CpaInstanceHandle instanceHandle,
               const CpaCyGenFlatBufCbFunc pKeyGenCb,
               void *pCallbackTag,
               const CpaCyKeyGenMgfOpData *pKeyGenMgfOpData,
               CpaFlatBuffer *pGeneratedMaskBuffer)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    icp_qat_fw_la_key_gen_req_t keyGenReq;
    lac_sym_qat_content_desc_info_t contentDescInfo = {0};
    lac_sym_key_cookie_t *pCookie = NULL;
    Cpa64U inputPhysAddr = 0;
    Cpa64U outputPhysAddr = 0;
    CpaCySymHashSetupData hashSetupData = {0};
    Cpa32U hashBlkSizeInBytes = 0;

    /* If synchronous Operation */
    if (NULL == pKeyGenCb)
    {
        return LacSymKey_MgfSync(instanceHandle, pKeyGenCb, pCallbackTag, 
                                pKeyGenMgfOpData, pGeneratedMaskBuffer); 
    }

#ifdef ICP_PARAM_CHECK
    LAC_INITIALISED_CHECK();
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);
    LAC_CHECK_NULL_PARAM(pKeyGenMgfOpData);
    LAC_CHECK_NULL_PARAM(pGeneratedMaskBuffer);
    LAC_CHECK_NULL_PARAM(pGeneratedMaskBuffer->pData);
    LAC_CHECK_NULL_PARAM(pKeyGenMgfOpData->seedBuffer.pData);

    LAC_ASSERT_NOT_NULL(pKeyGenCb);

    /* Maximum seed length for MGF1 request */
    if (pKeyGenMgfOpData->seedBuffer.dataLenInBytes >
                   ICP_QAT_FW_LA_MGF_SEED_LEN_MAX)
    {
        LAC_INVALID_PARAM_LOG("seedBuffer.dataLenInBytes");
        return CPA_STATUS_INVALID_PARAM;
    }

    /* Maximum mask length for MGF1 request */
    if (pKeyGenMgfOpData->maskLenInBytes > ICP_QAT_FW_LA_MGF_MASK_LEN_MAX)
    {
        LAC_INVALID_PARAM_LOG("maskLenInBytes");
        return CPA_STATUS_INVALID_PARAM;
    }

    /* check for enough space in the flat buffer */
    if (pKeyGenMgfOpData->maskLenInBytes > 
        pGeneratedMaskBuffer->dataLenInBytes)
    {
        LAC_INVALID_PARAM_LOG("pGeneratedMaskBuffer.dataLenInBytes");
        return CPA_STATUS_INVALID_PARAM;
    }
#endif //ICP_PARAM_CHECK

    /* Allocate the cookie */
    if((pCookie =
        (lac_sym_key_cookie_t*)Lac_MemPoolEntryAlloc(lac_sym_cookie_memory))
        == NULL)
    {
        LAC_LOG_ERROR("Cannot get mem pool entry");
        status = CPA_STATUS_RESOURCE;
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        /* populate the cookie */
        pCookie->instanceHandle= instanceHandle;
        pCookie->pCallbackTag = pCallbackTag;
        pCookie->pKeyGenOpData =
            (CpaCyKeyGenMgfOpData *) LAC_CONST_PTR_CAST(pKeyGenMgfOpData);
        pCookie->pKeyGenCb = pKeyGenCb;
        hashSetupData.hashAlgorithm = CPA_CY_SYM_HASH_SHA1;
        hashSetupData.hashMode = CPA_CY_SYM_HASH_MODE_PLAIN;
        hashSetupData.digestResultLenInBytes = LAC_HASH_SHA1_DIGEST_SIZE;

        /* Populate the Control block of the content descriptor */
        LacSymQat_HashContentDescInit(
                &hashSetupData,
                /* control block first in content descriptor */
                (icp_qat_fw_auth_hdr_t *)pCookie->contentDesc,
                /* point to base of hw setup block */
                (Cpa8U *)pCookie->contentDesc + sizeof(icp_qat_fw_auth_hdr_t),
                LAC_SYM_KEY_NO_HASH_BLK_OFFSET_BYTES,
                ICP_QAT_FW_SLICE_NULL,
                ICP_QAT_HW_AUTH_MODE0,  /* just a plain hash */
                NULL,
                &hashBlkSizeInBytes);

        LacSymQat_KeyMgfRequestPopulate(
            &keyGenReq,
            pKeyGenMgfOpData->seedBuffer.dataLenInBytes,
            pKeyGenMgfOpData->maskLenInBytes,
            LAC_HASH_SHA1_DIGEST_SIZE);

        contentDescInfo.pData = pCookie->contentDesc;
        contentDescInfo.pDataPhys = LAC_MEM_CAST_PTR_TO_UINT64(
                LAC_OS_VIRT_TO_PHYS(pCookie->contentDesc));
        contentDescInfo.hdrSzQuadWords =
            LAC_BYTES_TO_QUADWORDS(sizeof(icp_qat_fw_auth_hdr_t));
        contentDescInfo.hwBlkSzQuadWords =
            LAC_BYTES_TO_QUADWORDS(hashBlkSizeInBytes);

        /* Populate common request fields */
        inputPhysAddr = LAC_MEM_CAST_PTR_TO_UINT64(
                LAC_OS_VIRT_TO_PHYS(pKeyGenMgfOpData->seedBuffer.pData));

        outputPhysAddr = LAC_MEM_CAST_PTR_TO_UINT64(
                LAC_OS_VIRT_TO_PHYS(pGeneratedMaskBuffer->pData));

        LacSymQat_ComnReqHdrPopulate(
            &(keyGenReq.comn_req),
            &contentDescInfo,
            pCookie,
            inputPhysAddr,
            outputPhysAddr,
            CPA_FALSE, /* not ordered */
            LAC_SYM_QAT_FLAT_BUFFER);

        status = QatComms_ReqHdrCreate(&keyGenReq, ICP_ARCH_IF_REQ_QAT_FW_LA);
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        /* Send the message to the QAT */
        status = QatComms_MsgSend(
            &keyGenReq,
            ICP_ARCH_IF_REQ_QAT_FW_LA,
            QAT_COMMS_PRIORITY_NORMAL,
            instanceHandle);
    }

    /* Update stats */
    if (CPA_STATUS_SUCCESS == status)
    {
        LAC_KEY_STAT_INC(numMgfKeyGenRequests, instanceHandle);
    }
    else
    {
        LAC_KEY_STAT_INC(numMgfKeyGenRequestErrors, instanceHandle);

        /* clean up memory */
        if (NULL != pCookie)
        {
            Lac_MemPoolEntryFree(pCookie);
        }
    }

    return status;
}


/**
 ******************************************************************************
 * @ingroup LacSymKey
 *      Key Generation SSL & TLS response handler
 *
 * @description
 *      Handles Key Generation SSL & TLS response messages from the QAT.
 *
 * @param[in] lacCmdId        Command id of the original request
 * @param[in] pOpaqueData     Pointer to opaque data that was in request
 * @param[in] cmnRespFlags    LA response flags
 *
 * @return void
 *
 *****************************************************************************/
STATIC void
LacSymKey_SslTlsHandleResponse(icp_qat_fw_la_cmd_id_t lacCmdId,
    void *pOpaqueData,
    icp_qat_fw_comn_flags cmnRespFlags)
{
    void *pSslTlsOpData = NULL;
    CpaCyGenFlatBufCbFunc pKeyGenSslTlsCb = NULL;
    lac_sym_key_cookie_t *pCookie = NULL;
    void *pCallbackTag = NULL;
    CpaFlatBuffer *pGeneratedKeyBuffer= NULL;
    CpaStatus status = CPA_STATUS_SUCCESS;

    CpaBoolean respStatusOk = (ICP_QAT_FW_COMN_STATUS_FLAG_OK ==
        ICP_QAT_FW_COMN_STATUS_GET(cmnRespFlags)) ? CPA_TRUE : CPA_FALSE;

    pCookie = (lac_sym_key_cookie_t *)pOpaqueData;

    pSslTlsOpData = pCookie->pKeyGenOpData;

    if (CPA_TRUE == respStatusOk)
    {
        if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.key.istat)
        {
            LacKey_StatsInc(lacCmdId,
                            LAC_KEY_COMPLETED,
                            pCookie->instanceHandle);
        } /* endif collectStatistics */
    }
    else
    {
        status = CPA_STATUS_FAIL;

        if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.key.istat)
        {
            LacKey_StatsInc(lacCmdId,
                            LAC_KEY_COMPLETED_ERRORS,
                            pCookie->instanceHandle);
        } /* endif collectStatistics */
    }

    pKeyGenSslTlsCb = (CpaCyGenFlatBufCbFunc)(pCookie->pKeyGenCb);

    LAC_ASSERT_NOT_NULL(pKeyGenSslTlsCb);

    pCallbackTag = pCookie->pCallbackTag;
    pGeneratedKeyBuffer = (CpaFlatBuffer*) pCookie->pKeyGenOpData;

    Lac_MemPoolEntryFree(pCookie);

    (*pKeyGenSslTlsCb)(pCallbackTag, status,
                        pSslTlsOpData, pGeneratedKeyBuffer);
}


STATIC CpaStatus
LacSymKey_SslTlsSync(
    CpaInstanceHandle instanceHandle,
    const CpaCyGenFlatBufCbFunc pKeyGenCb,
    void *pCallbackTag,
    icp_qat_fw_la_cmd_id_t lacCmdId,
    void *pKeyGenSslTlsOpData,
    Cpa8U *pData)
{
    lac_sync_op_data_t *pSyncCallbackData = NULL;
    CpaStatus status = CPA_STATUS_SUCCESS;

    status = LacSync_CreateSyncCookie(&pSyncCallbackData);

    if (CPA_STATUS_SUCCESS == status)
    {
        status = LacKeyGenSslTls_GenCommon(instanceHandle,
                                           pKeyGenCb,
                                           pSyncCallbackData,
                                           lacCmdId,
                                           pKeyGenSslTlsOpData,
                                           pData);
    }
    else
    {
        /* Failure allocating sync cookie */
        if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.key.istat)
        {
            LacKey_StatsInc(lacCmdId,
                            LAC_KEY_REQUEST_ERRORS,
                            instanceHandle);
        }
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        CpaStatus syncStatus = CPA_STATUS_SUCCESS;

        syncStatus = LacSync_WaitForCallback(pSyncCallbackData,
                                             LAC_SYM_SYNC_CALLBACK_TIMEOUT,
                                             &status,
                                             NULL);

        /* If callback doesn't come back */
        if (CPA_STATUS_SUCCESS != syncStatus)
        {
            if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.key.istat)
            {
                LacKey_StatsInc(lacCmdId,
                                LAC_KEY_COMPLETED_ERRORS,
                                instanceHandle);
            }
            LAC_LOG_ERROR("Callback timed out");
            status = syncStatus;
        }
    }

    LacSync_DestroySyncCookie(&pSyncCallbackData);

    return status;
}


CpaStatus
LacKeyGenSslTls_GenCommon(
    CpaInstanceHandle instanceHandle,
    const CpaCyGenFlatBufCbFunc pKeyGenCb,
    void *pCallbackTag,
    icp_qat_fw_la_cmd_id_t lacCmdId,
    void *pKeyGenSslTlsOpData,
    Cpa8U *pData)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    icp_qat_fw_la_key_gen_req_t keyGenReq;
    lac_sym_key_cookie_t *pCookie = NULL;
    Cpa64U inputPhysAddr = 0;
    Cpa64U outputPhysAddr = 0;
    CpaCySymHashSetupData hashSetupData = {0};
    Cpa32U hashBlkSizeInBytes = 0;
    lac_sym_qat_content_desc_info_t contentDescInfo = {0};
    Cpa32U tlsPrefixLen = 0;
    CpaCySymHashNestedModeSetupData *pNestedModeSetupData =
            &(hashSetupData.nestedModeSetupData);

    /* If synchronous Operation */
    if (NULL == pKeyGenCb)
    {
        return LacSymKey_SslTlsSync(instanceHandle, LacSync_GenFlatBufCb, 
                    pCallbackTag, lacCmdId, pKeyGenSslTlsOpData, pData);
    }

/* Allocate the cookie */
    if((pCookie =
        (lac_sym_key_cookie_t*)Lac_MemPoolEntryAlloc(lac_sym_cookie_memory))
        == NULL)
    {
        LAC_LOG_ERROR("Cannot get mem pool entry");
        status = CPA_STATUS_RESOURCE;
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        icp_qat_hw_auth_mode_t qatHashMode = 0;

        if (ICP_QAT_FW_LA_CMD_SSL3_KEY_DERIVE == lacCmdId)
        {
            qatHashMode = ICP_QAT_HW_AUTH_MODE0;
        }
        else
        {
            qatHashMode = ICP_QAT_HW_AUTH_MODE2;
        }

        pCookie->instanceHandle = instanceHandle;
        pCookie->pCallbackTag = pCallbackTag;
        pCookie->pKeyGenCb = pKeyGenCb;
        pCookie->pKeyGenOpData = pKeyGenSslTlsOpData;

        hashSetupData.hashMode = CPA_CY_SYM_HASH_MODE_NESTED;
        hashSetupData.hashAlgorithm = CPA_CY_SYM_HASH_SHA1;
        hashSetupData.digestResultLenInBytes = LAC_HASH_MD5_DIGEST_SIZE;
        pNestedModeSetupData->outerHashAlgorithm = CPA_CY_SYM_HASH_MD5;

        if (ICP_QAT_FW_LA_CMD_SSL3_KEY_DERIVE == lacCmdId)
        {
            pNestedModeSetupData->pInnerPrefixData = NULL;
            pNestedModeSetupData->innerPrefixLenInBytes = 0;
            pNestedModeSetupData->pOuterPrefixData = NULL;
            pNestedModeSetupData->outerPrefixLenInBytes = 0;
        }
        else
        {
            CpaCyKeyGenTlsOpData *pKeyGenTlsOpData =
                (CpaCyKeyGenTlsOpData *)pKeyGenSslTlsOpData;

            /* secret = [s1 | s2 ]
             * s1 = outer prefix, s2 = inner prefix
             * length of s1 and s2 = ceil(secret_length / 2) */

            /* (secret length + 1)/2 will always give the ceil as division by 2
             * (>>1) will give the smallest integral value not less than arg */
            tlsPrefixLen =
                (pKeyGenTlsOpData->secret.dataLenInBytes + 1) >> 1;

            /* last byte of s1 will be first byte of s2 if Length is odd */
            pNestedModeSetupData->pInnerPrefixData =
                pKeyGenTlsOpData->secret.pData +
                (pKeyGenTlsOpData->secret.dataLenInBytes - tlsPrefixLen);

            pNestedModeSetupData->pOuterPrefixData =
                pKeyGenTlsOpData->secret.pData;

            pNestedModeSetupData->innerPrefixLenInBytes = tlsPrefixLen;
            pNestedModeSetupData->outerPrefixLenInBytes = tlsPrefixLen;
        }

        /* note that following function doesn't look at inner/outer
         * prefix pointers in nested digest ctx */
        LacSymQat_HashContentDescInit(
                &hashSetupData,
                /* control block is first in content descriptor */
                (icp_qat_fw_auth_hdr_t *)pCookie->contentDesc,
                /* pointer to base of hw setup block */
                pCookie->contentDesc + sizeof(icp_qat_fw_auth_hdr_t),
                LAC_SYM_KEY_NO_HASH_BLK_OFFSET_BYTES,
                ICP_QAT_FW_SLICE_NULL,
                qatHashMode,
                NULL, /* precompute data */
                &hashBlkSizeInBytes);


        if (ICP_QAT_FW_LA_CMD_SSL3_KEY_DERIVE == lacCmdId)
        {
            CpaCyKeyGenSslOpData *pKeyGenSslOpData =
                (CpaCyKeyGenSslOpData *)pKeyGenSslTlsOpData;
            Cpa8U *pLabel = NULL;
            Cpa32U labelLen = 0;
            Cpa8U iterations = 0;

            /* iterations = ceil of output required / output per iteration
             * Ceil of a / b = (a + (b-1)) / b */
            iterations = (pKeyGenSslOpData->generatedKeyLenInBytes + 
                            (LAC_SYM_QAT_KEY_SSL_BYTES_PER_ITERATION - 1))
                            >> LAC_SYM_QAT_KEY_SSL_ITERATIONS_SHIFT;

            if (CPA_CY_KEY_SSL_OP_USER_DEFINED == pKeyGenSslOpData->sslOp)
            {
                pLabel = pKeyGenSslOpData->userLabel.pData;
                labelLen = pKeyGenSslOpData->userLabel.dataLenInBytes;
            }
            else
            {
                pLabel = pSslLabel;

                /* calculate label length.
                 * eg. 3 iterations is ABBCCC so length is 6 */
                labelLen = ((iterations * iterations) + iterations) >> 1;
            }

            LacSymQat_KeySslRequestPopulate(
                &keyGenReq,
                pKeyGenSslOpData->generatedKeyLenInBytes,
                labelLen,
                pKeyGenSslOpData->secret.dataLenInBytes,
                iterations);

            LacSymQat_KeySslKeyMaterialInputPopulate(
                &(pCookie->u.sslKeyInput),
                pKeyGenSslOpData->seed.pData,
                pLabel,
                pKeyGenSslOpData->secret.pData);

            inputPhysAddr = LAC_MEM_CAST_PTR_TO_UINT64(
                LAC_OS_VIRT_TO_PHYS(&(pCookie->u.sslKeyInput)));

        }
        else
        {
            CpaCyKeyGenTlsOpData *pKeyGenTlsOpData =
                (CpaCyKeyGenTlsOpData *)pKeyGenSslTlsOpData;
            lac_sym_qat_hash_state_buffer_info_t hashStateBufferInfo = {0};
            icp_qat_fw_auth_hdr_t *pHashControlBlock = 
                (icp_qat_fw_auth_hdr_t *)pCookie->contentDesc;
            Cpa8U *pLabel = NULL;
            Cpa32U labelLen = 0;

            hashStateBufferInfo.stateStorageSzQuadWords = 0; 
            /* Prefix sizes are block sizes of algorithms so are already 
             * quad word multiples */
            hashStateBufferInfo.prefixAadSzQuadWords = LAC_BYTES_TO_QUADWORDS(
                    pHashControlBlock->u.inner_prefix_sz +  
                    pHashControlBlock->outer_prefix_sz);
            hashStateBufferInfo.pData = pCookie->hashStateBuffer;
            hashStateBufferInfo.pDataPhys = LAC_MEM_CAST_PTR_TO_UINT64(
                LAC_OS_VIRT_TO_PHYS(&(pCookie->hashStateBuffer)));

            /* Copy prefix data into hash state buffer */
            LacSymQat_HashStatePrefixAadBufferPopulate(
                &hashStateBufferInfo,
                (icp_qat_fw_auth_hdr_t *)pCookie->contentDesc,
                hashSetupData.nestedModeSetupData.pInnerPrefixData,
                pNestedModeSetupData->innerPrefixLenInBytes,
                hashSetupData.nestedModeSetupData.pOuterPrefixData,
                pNestedModeSetupData->outerPrefixLenInBytes);

            /* Firmware only looks at hash state buffer pointer and the
             * hash state buffer size so all other fields are set to 0 */
            LacSymQat_HashRequestParamsPopulate(
                &(pCookie->hashRequestParams),
                ICP_QAT_FW_SLICE_NULL,
                &hashStateBufferInfo, /* hash state prefix buffer */
                ICP_QAT_FW_LA_PARTIAL_NONE,
                0, /* hash result size */
                0, /* auth offset */
                0, /* auth len*/
                CPA_FALSE,
                NULL);

            /* Set up the labels and their length */
            if (CPA_CY_KEY_TLS_OP_USER_DEFINED == pKeyGenTlsOpData->tlsOp)
            {
                pLabel = pKeyGenTlsOpData->userLabel.pData;
                labelLen = pKeyGenTlsOpData->userLabel.dataLenInBytes;
            }
            else if (CPA_CY_KEY_TLS_OP_MASTER_SECRET_DERIVE ==
                     pKeyGenTlsOpData->tlsOp)
            {
                pLabel = pTlsLabel->masterSecret;
                labelLen = sizeof(LAC_SYM_KEY_TLS_MASTER_SECRET_LABEL) - 1;
            }
            else if (CPA_CY_KEY_TLS_OP_KEY_MATERIAL_DERIVE ==
                     pKeyGenTlsOpData->tlsOp)
            {
                pLabel = pTlsLabel->keyMaterial;
                labelLen = sizeof(LAC_SYM_KEY_TLS_KEY_MATERIAL_LABEL) - 1;
            }
            else if (CPA_CY_KEY_TLS_OP_CLIENT_FINISHED_DERIVE ==
                     pKeyGenTlsOpData->tlsOp)
            {
                pLabel = pTlsLabel->clientFinished;
                labelLen = sizeof(LAC_SYM_KEY_TLS_CLIENT_FIN_LABEL) - 1;
            }
            else
            {
                pLabel = pTlsLabel->serverFinished;
                labelLen = sizeof(LAC_SYM_KEY_TLS_SERVER_FIN_LABEL) - 1;
            }

            LacSymQat_KeyTlsRequestPopulate(
                &keyGenReq,
                &(pCookie->hashRequestParams),
                pKeyGenTlsOpData->generatedKeyLenInBytes,
                labelLen,
                pKeyGenTlsOpData->secret.dataLenInBytes);

            LacSymQat_KeyTlsKeyMaterialInputPopulate(
                &(pCookie->u.tlsKeyInput),
                pKeyGenTlsOpData->seed.pData,
                pLabel);

            inputPhysAddr = LAC_MEM_CAST_PTR_TO_UINT64(
                LAC_OS_VIRT_TO_PHYS(&(pCookie->u.tlsKeyInput)));
        }

        outputPhysAddr = LAC_MEM_CAST_PTR_TO_UINT64(
            LAC_OS_VIRT_TO_PHYS(pData));

        contentDescInfo.pData = pCookie->contentDesc;
        contentDescInfo.pDataPhys = LAC_MEM_CAST_PTR_TO_UINT64(
            LAC_OS_VIRT_TO_PHYS(pCookie->contentDesc));
        contentDescInfo.hdrSzQuadWords =
                    LAC_BYTES_TO_QUADWORDS(sizeof(icp_qat_fw_auth_hdr_t));
        contentDescInfo.hwBlkSzQuadWords =
                    LAC_BYTES_TO_QUADWORDS(hashBlkSizeInBytes);
        /* Populate common request fields */

        LacSymQat_ComnReqHdrPopulate(
            &(keyGenReq.comn_req),
            &contentDescInfo,
            pCookie,
            inputPhysAddr,
            outputPhysAddr,
            CPA_FALSE, /* not ordered */
            LAC_SYM_QAT_FLAT_BUFFER);

        status = QatComms_ReqHdrCreate(&keyGenReq, ICP_ARCH_IF_REQ_QAT_FW_LA);
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        /* Send message to QAT */
        status = QatComms_MsgSend(
            &keyGenReq,
            ICP_ARCH_IF_REQ_QAT_FW_LA,
            QAT_COMMS_PRIORITY_NORMAL,
            instanceHandle);
    }

    /* Update stats */
    if (CPA_STATUS_SUCCESS == status)
    {
        if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.key.istat)
        {
            LacKey_StatsInc(lacCmdId,
                            LAC_KEY_REQUESTS,
                            pCookie->instanceHandle);
        }
    }
    else
    {
        /* clean up cookie memory */
        if (NULL != pCookie)
        {
            if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.key.istat)
            {
                LacKey_StatsInc(lacCmdId,
                                LAC_KEY_REQUEST_ERRORS,
                                pCookie->instanceHandle);
            }
            Lac_MemPoolEntryFree(pCookie);
        }
    }

    return status;
}


/**
* cpaCyKeyGenSsl
*/
CpaStatus
cpaCyKeyGenSsl(const CpaInstanceHandle instanceHandle,
               const CpaCyGenFlatBufCbFunc pKeyGenCb,
               void *pCallbackTag,
               const CpaCyKeyGenSslOpData *pKeyGenSslOpData,
               CpaFlatBuffer *pGeneratedKeyBuffer)
{
#ifdef ICP_PARAM_CHECK
    LAC_INITIALISED_CHECK();

    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);
    LAC_CHECK_NULL_PARAM(pKeyGenSslOpData);
    LAC_CHECK_NULL_PARAM(pKeyGenSslOpData->secret.pData);
    LAC_CHECK_NULL_PARAM(pKeyGenSslOpData->seed.pData);
    LAC_CHECK_NULL_PARAM(pGeneratedKeyBuffer);
    LAC_CHECK_NULL_PARAM(pGeneratedKeyBuffer->pData);

    /* Maximum secret length for SSL3 Key Gen request */
    if (pKeyGenSslOpData->secret.dataLenInBytes >
                            ICP_QAT_FW_LA_SSL_SECRET_LEN_MAX)
    {
        LAC_INVALID_PARAM_LOG("secret.dataLenInBytes");
        return CPA_STATUS_INVALID_PARAM;
    }

    switch(pKeyGenSslOpData->sslOp)
    {
        case CPA_CY_KEY_SSL_OP_MASTER_SECRET_DERIVE:
        case CPA_CY_KEY_SSL_OP_KEY_MATERIAL_DERIVE:
            break;
        case CPA_CY_KEY_SSL_OP_USER_DEFINED:
        {
            LAC_CHECK_NULL_PARAM(pKeyGenSslOpData->userLabel.pData);

            /* Maximum label length for SSL3 Key Gen request */
            if (pKeyGenSslOpData->userLabel.dataLenInBytes >
                ICP_QAT_FW_LA_SSL_LABEL_LEN_MAX)
            {
                LAC_INVALID_PARAM_LOG("userLabel.dataLenInBytes");
                return CPA_STATUS_INVALID_PARAM;
            }

            break;
        }
        default:
        {
            LAC_INVALID_PARAM_LOG("sslOp");
            return CPA_STATUS_INVALID_PARAM;
        }
    }

    /* seed length for SSL3 Key Gen request */
    if (ICP_QAT_FW_LA_SSL_SEED_LEN_MAX != pKeyGenSslOpData->seed.dataLenInBytes)
    {
        LAC_INVALID_PARAM_LOG("seed.dataLenInBytes");
        return CPA_STATUS_INVALID_PARAM;
    }

    /* Maximum output length for SSL3 Key Gen request */
    if (pKeyGenSslOpData->generatedKeyLenInBytes >
        ICP_QAT_FW_LA_SSL_OUTPUT_LEN_MAX)
    {
        LAC_INVALID_PARAM_LOG("generatedKeyLenInBytes");
        return CPA_STATUS_INVALID_PARAM;
    }

    /* Check for enough space in the flat buffer */
    if (pKeyGenSslOpData->generatedKeyLenInBytes > 
        pGeneratedKeyBuffer->dataLenInBytes)
    {
        LAC_INVALID_PARAM_LOG("pGeneratedKeyBuffer->dataLenInBytes");
        return CPA_STATUS_INVALID_PARAM;
    }
#endif //ICP_PARAM_CHECK

    return LacKeyGenSslTls_GenCommon(
        instanceHandle,
        pKeyGenCb,
        pCallbackTag,
        ICP_QAT_FW_LA_CMD_SSL3_KEY_DERIVE,
        LAC_CONST_PTR_CAST(pKeyGenSslOpData),
        pGeneratedKeyBuffer->pData);
}


/**
* cpaCyKeyGenTls
*/
CpaStatus
cpaCyKeyGenTls(const CpaInstanceHandle instanceHandle,
               const CpaCyGenFlatBufCbFunc pKeyGenCb,
               void *pCallbackTag,
               const CpaCyKeyGenTlsOpData *pKeyGenTlsOpData,
               CpaFlatBuffer *pGeneratedKeyBuffer)
{
#ifdef ICP_PARAM_CHECK
    LAC_INITIALISED_CHECK();

    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);
    LAC_CHECK_NULL_PARAM(pKeyGenTlsOpData);
    LAC_CHECK_NULL_PARAM(pKeyGenTlsOpData->secret.pData);
    LAC_CHECK_NULL_PARAM(pKeyGenTlsOpData->seed.pData);
    LAC_CHECK_NULL_PARAM(pGeneratedKeyBuffer);
    LAC_CHECK_NULL_PARAM(pGeneratedKeyBuffer->pData);

    /* Maximum secret length for TLS Key Gen request */
    if (pKeyGenTlsOpData->secret.dataLenInBytes >
                            ICP_QAT_FW_LA_TLS_SECRET_LEN_MAX)
    {
        LAC_INVALID_PARAM_LOG("secret.dataLenInBytes");
        return CPA_STATUS_INVALID_PARAM;
    }

    switch (pKeyGenTlsOpData->tlsOp)
    {
        case CPA_CY_KEY_TLS_OP_MASTER_SECRET_DERIVE:
        case CPA_CY_KEY_TLS_OP_KEY_MATERIAL_DERIVE:
        case CPA_CY_KEY_TLS_OP_CLIENT_FINISHED_DERIVE:
        case CPA_CY_KEY_TLS_OP_SERVER_FINISHED_DERIVE:
            break;
        case CPA_CY_KEY_TLS_OP_USER_DEFINED:
        {
            LAC_CHECK_NULL_PARAM(pKeyGenTlsOpData->userLabel.pData);

            /* Maximum label length for TLS Key Gen request */
            if (pKeyGenTlsOpData->userLabel.dataLenInBytes >
                ICP_QAT_FW_LA_TLS_LABEL_LEN_MAX)
            {
                LAC_INVALID_PARAM_LOG("userLabel.dataLenInBytes");
                return CPA_STATUS_INVALID_PARAM;
            }
            break;
        }
        default:
        {
            LAC_INVALID_PARAM_LOG("tlsOp");
            return CPA_STATUS_INVALID_PARAM;
        }
    }

    /* only seed length for TLS Key Gen request */
    if (ICP_QAT_FW_LA_TLS_SEED_LEN_MAX != pKeyGenTlsOpData->seed.dataLenInBytes)
    {
        LAC_INVALID_PARAM_LOG("seed.dataLenInBytes");
        return CPA_STATUS_INVALID_PARAM;
    }

    /* Maximum output length for TLS Key Gen request */
    if (pKeyGenTlsOpData->generatedKeyLenInBytes >
        ICP_QAT_FW_LA_TLS_OUTPUT_LEN_MAX)
    {
        LAC_INVALID_PARAM_LOG("generatedKeyLenInBytes");
        return CPA_STATUS_INVALID_PARAM;
    }

    /* Check for enough space in the flat buffer */
    if (pKeyGenTlsOpData->generatedKeyLenInBytes > 
        pGeneratedKeyBuffer->dataLenInBytes)
    {
        LAC_INVALID_PARAM_LOG("pGeneratedKeyBuffer->dataLenInBytes");
        return CPA_STATUS_INVALID_PARAM;
    }
#endif //ICP_PARAM_CHECK

    return LacKeyGenSslTls_GenCommon(
        instanceHandle,
        pKeyGenCb,
        pCallbackTag,
        ICP_QAT_FW_LA_CMD_TLS_KEY_DERIVE,
        LAC_CONST_PTR_CAST(pKeyGenTlsOpData),
        pGeneratedKeyBuffer->pData);
}


/*
 * LacSymKey_Init
 */
CpaStatus
LacSymKey_Init(void)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    LAC_OS_BZERO(&lacKeyStats, sizeof(lacKeyStats));

    status = LAC_OS_CAMALLOC(&pSslLabel, 
                ICP_QAT_FW_LA_SSL_LABEL_LEN_MAX, 
                LAC_8BYTE_ALIGNMENT);

    if (CPA_STATUS_SUCCESS == status)
    {
        Cpa32U i = 0;
        Cpa32U offset = 0;

        /* Initialise SSL label ABBCCC..... */
        for (i = 0; i < ICP_QAT_FW_LA_SSL_ITERATES_LEN_MAX; i++)
        {
            ixOsalMemSet(pSslLabel + offset, 'A' + i, i + 1);
            offset += (i + 1);
        }
    
        /* allocate memory for TLS labels */
        status = LAC_OS_CAMALLOC(&pTlsLabel, 
                    sizeof(lac_sym_key_tls_labels_t), 
                    LAC_8BYTE_ALIGNMENT);
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        LAC_OS_BZERO(pTlsLabel, sizeof(lac_sym_key_tls_labels_t));

        /* copy the TLS labels into the dynamically allocated structure  */
        memcpy(pTlsLabel->masterSecret, LAC_SYM_KEY_TLS_MASTER_SECRET_LABEL,
                sizeof(LAC_SYM_KEY_TLS_MASTER_SECRET_LABEL) - 1);

        memcpy(pTlsLabel->keyMaterial, LAC_SYM_KEY_TLS_KEY_MATERIAL_LABEL,
                sizeof(LAC_SYM_KEY_TLS_KEY_MATERIAL_LABEL) - 1);

        memcpy(pTlsLabel->clientFinished, LAC_SYM_KEY_TLS_CLIENT_FIN_LABEL,
                sizeof(LAC_SYM_KEY_TLS_CLIENT_FIN_LABEL) - 1);

        memcpy(pTlsLabel->serverFinished, LAC_SYM_KEY_TLS_SERVER_FIN_LABEL,
                sizeof(LAC_SYM_KEY_TLS_SERVER_FIN_LABEL) - 1);

        LacSymQat_RespHandlerRegister(
            ICP_QAT_FW_LA_CMD_SSL3_KEY_DERIVE,
            LacSymKey_SslTlsHandleResponse);

        LacSymQat_RespHandlerRegister(
            ICP_QAT_FW_LA_CMD_TLS_KEY_DERIVE,
            LacSymKey_SslTlsHandleResponse);

        LacSymQat_RespHandlerRegister(
            ICP_QAT_FW_LA_CMD_MGF1,
            LacSymKey_MgfHandleResponse);
    }

    if (CPA_STATUS_SUCCESS != status)
    {
        LAC_OS_CAFREE(pSslLabel, ICP_QAT_FW_LA_SSL_LABEL_LEN_MAX);
        LAC_OS_CAFREE(pTlsLabel, sizeof(lac_sym_key_tls_labels_t));
    }

    return status;
}


/*
 * LacSymKey_Shutdown
 */
CpaStatus
LacSymKey_Shutdown(void)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    LAC_OS_BZERO(&lacKeyStats, sizeof(lacKeyStats));

    LAC_OS_CAFREE(pSslLabel, ICP_QAT_FW_LA_SSL_LABEL_LEN_MAX);
    LAC_OS_CAFREE(pTlsLabel, sizeof(lac_sym_key_tls_labels_t));

    return status;
}

