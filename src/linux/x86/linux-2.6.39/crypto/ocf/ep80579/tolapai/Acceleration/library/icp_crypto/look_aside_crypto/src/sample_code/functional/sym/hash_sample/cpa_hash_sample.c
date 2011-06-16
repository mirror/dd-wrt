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
 * cpa_hash_sample.c
 * 

 * sampleHashFunctional
 *
 * sampleCode
 * 
 * This is sample code that uses Hash APIs. A sample text is hashed with MD5
 * algorithm.
 * 
 *****************************************************************************/

#include "cpa.h"
#include "cpa_cy_im.h"
#include "cpa_cy_sym.h"



#include "../../include/cpa_sample_utils.h"

#define DIGEST_LENGTH 16
#define TIMEOUT_MS 5000 /* 5 seconds*/


extern int debugParam;

static Cpa8U vectorData[] = {
        0xD7,0x1B,0xA4,0xCA,0xEC,0xBD,0x15,0xE2,0x52,0x6A,0x21,0x0B,
        0x81,0x77,0x0C,0x90,0x68,0xF6,0x86,0x50,0xC6,0x2C,0x6E,0xED,
        0x2F,0x68,0x39,0x71,0x75,0x1D,0x94,0xF9,0x0B,0x21,0x39,0x06,
        0xBE,0x20,0x94,0xC3,0x43,0x4F,0x92,0xC9,0x07,0xAA,0xFE,0x7F,
        0xCF,0x05,0x28,0x6B,0x82,0xC4,0xD7,0x5E,0xF3,0xC7,0x74,0x68,
        0xCF,0x05,0x28,0x6B,0x82,0xC4,0xD7,0x5E,0xF3,0xC7,0x74,0x68,
        0x80,0x8B,0x28,0x8D,0xCD,0xCA,0x94,0xB8,0xF5,0x66,0x0C,0x00,
        0x5C,0x69,0xFC,0xE8,0x7F,0x0D,0x81,0x97,0x48,0xC3,0x6D,0x24
};

static Cpa8U digest[] = {
        0x98, 0xD3, 0xED, 0x1A, 0xF3, 0x3A, 0xD3, 0x7A,
        0x97, 0x9F, 0xAE, 0x55, 0xBC, 0xDB, 0xA2, 0x68
};

/*****************************************************************************
 * sampleHashFunctional
 *****************************************************************************/
void
hashSample(void);

/**
 *****************************************************************************
 * sampleHashFunctional
 * Symmetric callback function
 * It is a signal that the operation is completed. The user can call the
 * application above, free the memory, etc.
 * In this example, the function only sets the complete variable to indicate
 * it has been called
 *
 *****************************************************************************/
static void
symCallback(void *pCallbackTag,
        CpaStatus status,
        const CpaCySymOp operationType,
        void *pOpData,
        CpaBufferList *pDstBuffer,
        CpaBoolean verifyResult)
{
    PRINT_DBG("Callback called with status = %d.\n", status);

    if (NULL != pCallbackTag) 
    {

        /** indicate that the function has been called*/
        COMPLETE((struct COMPLETION_STRUCT *)pCallbackTag);
    }
}

/**
 *****************************************************************************
 * sampleHashFunctional
 * Perform an hash operation
 *
 *****************************************************************************/
static CpaStatus 
hashPerformOp(CpaCySymSessionCtx pSessionCtx)
{
    CpaStatus       status          = CPA_STATUS_SUCCESS;
    Cpa8U           *pBufferMeta    = NULL;
    Cpa8U           *pSrcBuffer     = NULL;
    Cpa32U          bufferMetaSize  = 0;
    CpaBufferList   *pBufferList    = NULL;
    CpaFlatBuffer   *pFlatBuffer    = NULL;
    CpaCySymOpData  *pOpData        = NULL; 
    Cpa32U          bufferSize      = sizeof(vectorData) + DIGEST_LENGTH; 
    Cpa32U          numBuffers      = 1; /* only using 1 buffer in this case */
    Cpa32U        bufferListMemSize = sizeof(CpaBufferList) + 
                                      (numBuffers * sizeof(CpaFlatBuffer));

    /* allocate memory for bufferlist and array of flat buffers in a contiguous 
     * area and carve it up to reduce number of memory allocations required. */

    /* The following variables are allocated on the stack because we block
     * until the callback comes back. If a non-blocking approach was to be
     * used then these variables should be dynamically allocated */
    struct COMPLETION_STRUCT complete;
        COMPLETION_INIT((&complete));
    /* initialisation for callback; the "complete" variable is used by the
     * callback function to indicate it has been called*/

    /* get meta information size */
    PRINT_DBG("cpaCyBufferListGetMetaSize\n");
    status = cpaCyBufferListGetMetaSize( CPA_INSTANCE_HANDLE_SINGLE,
                numBuffers, &bufferMetaSize);
    
    if (CPA_STATUS_SUCCESS == status)
    {
        status = OS_MALLOC(&pBufferMeta, bufferMetaSize);
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        status = OS_MALLOC(&pBufferList, bufferListMemSize);
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        status = OS_MALLOC(&pFlatBuffer,sizeof(CpaFlatBuffer));
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        status = OS_MALLOC(&pSrcBuffer, bufferSize);
    }

    if (CPA_STATUS_SUCCESS == status) 
    {
        /* copy vector into buffer */
        memcpy(pSrcBuffer, vectorData, sizeof(vectorData));
        pBufferList->pBuffers = pFlatBuffer;
        pBufferList->numBuffers = 1;
        pBufferList->pPrivateMetaData = pBufferMeta;

        pFlatBuffer->dataLenInBytes = bufferSize;
        pFlatBuffer->pData = pSrcBuffer;     

        status = OS_MALLOC(&pOpData, sizeof(CpaCySymOpData));
    }




    if (CPA_STATUS_SUCCESS == status) 
    {
        pOpData->pSessionCtx = pSessionCtx;
        pOpData->packetType = CPA_CY_SYM_PACKET_TYPE_FULL;
        pOpData->hashStartSrcOffsetInBytes = 0;
        pOpData->messageLenToHashInBytes = sizeof(vectorData);
        pOpData->digestVerify = CPA_FALSE; 

        /* Place digest after data in the source buffer */
        pOpData->pDigestResult = pSrcBuffer + sizeof(vectorData);
    }

    if (CPA_STATUS_SUCCESS == status) 
    {
        PRINT_DBG("cpaCySymPerformOp\n");

        /** Perform symmetric operation */
        status = cpaCySymPerformOp(CPA_INSTANCE_HANDLE_SINGLE,
                (void *)&complete, /* data sent as is to the callback function*/
                pOpData,           /* operational data struct */
                pBufferList,       /* source buffer list */
                pBufferList,       /* same src & dst for an in-place operation*/
                NULL);    

        if (CPA_STATUS_SUCCESS != status) 
        {
            PRINT_ERR("cpaCySymPerformOp failed. (status = %d)\n", status);
        }

        if (CPA_STATUS_SUCCESS == status) 
        {
            /** wait until the completion of the operation*/
            if (!COMPLETION_WAIT((&complete), TIMEOUT_MS))
            {
                PRINT_ERR("timeout or interruption in cpaCySymPerformOp\n");
                status = CPA_STATUS_FAIL;
            }
        }

        if (CPA_STATUS_SUCCESS == status)
        {

            if (0 == memcmp(digest, pOpData->pDigestResult, DIGEST_LENGTH))
            {
                PRINT_DBG("Digest matches expected output\n");
            }
            else
            {
                PRINT_DBG("Digest does not match expected output\n");
                status = CPA_STATUS_FAIL;
            }
        }
    }
    
    /* At this stage, the callback function should have returned, 

     * so it is safe to free the memory */
    OS_FREE(pSrcBuffer);
    OS_FREE(pBufferList);
    OS_FREE(pBufferMeta);
    OS_FREE(pOpData);

    COMPLETION_DESTROY(&complete);

    return status;
}

void
hashSample(void)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    Cpa32U sessionCtxSize = 0;
    CpaCySymSessionCtx pSessionCtx = NULL;
#if defined(__linux)
    CpaCySymSessionSetupData sessionSetupData = {0};
    CpaCySymStats symStats = {0};
#elif defined(__freebsd)
    CpaCySymSessionSetupData sessionSetupData;
    CpaCySymStats symStats;
    memset((void *) &sessionSetupData, 0, sizeof(sessionSetupData));
    memset((void *) &symStats, 0, sizeof(symStats));
#endif

    /* Start Cryptographic component */
    PRINT_DBG("cpaCyStartInstance\n");
    status = cpaCyStartInstance(CPA_INSTANCE_HANDLE_SINGLE);



    if (CPA_STATUS_SUCCESS == status) 
    {
        /* populate symmetric session data structure 
         * for a plain hash operation */
        sessionSetupData.sessionPriority = CPA_CY_PRIORITY_NORMAL;
        sessionSetupData.symOperation = CPA_CY_SYM_OP_HASH;
        sessionSetupData.hashSetupData.hashAlgorithm = CPA_CY_SYM_HASH_MD5;
        sessionSetupData.hashSetupData.hashMode = CPA_CY_SYM_HASH_MODE_PLAIN;
        sessionSetupData.hashSetupData.digestResultLenInBytes = DIGEST_LENGTH;

        /* Determine size of session context to allocate */
        PRINT_DBG("cpaCySymSessionCtxGetSize\n");
        status = cpaCySymSessionCtxGetSize(CPA_INSTANCE_HANDLE_SINGLE, 
                    &sessionSetupData, &sessionCtxSize);
    }

    if (CPA_STATUS_SUCCESS == status) 
    {
        /* Allocate session context */
        status = OS_MALLOC(&pSessionCtx, sessionCtxSize);
    }

    if (CPA_STATUS_SUCCESS == status) 
    {
        /* Initialize the Hash session */
        PRINT_DBG("cpaCySymInitSession\n");
        status = cpaCySymInitSession(CPA_INSTANCE_HANDLE_SINGLE,
                    symCallback, &sessionSetupData, pSessionCtx);
    }

    if (CPA_STATUS_SUCCESS == status) 
    {
        CpaStatus sessionStatus = CPA_STATUS_SUCCESS;
 
        /* Perform Hash operation */
        status = hashPerformOp(pSessionCtx);

        /* Remove the session - session init has already succeeded */
        PRINT_DBG("cpaCySymRemoveSession\n");
        sessionStatus = cpaCySymRemoveSession(
                            CPA_INSTANCE_HANDLE_SINGLE, pSessionCtx);

        /* maintain status of remove session only when status of all operations
         * before it are successful. */
        if (CPA_STATUS_SUCCESS == status)
        {
            status = sessionStatus;
        }
    }

    if (CPA_STATUS_SUCCESS == status) 
    {
        /* Query symmetric statistics */
        /* NOTE: Stats can also be examined by the file /proc/icp-crypto/sym 
         * in the proc filesystem */
        status = cpaCySymQueryStats(CPA_INSTANCE_HANDLE_SINGLE, &symStats);

        if (CPA_STATUS_SUCCESS != status) 
        {
            PRINT_ERR("cpaCySymQueryStats failed, status = %d\n", status);
        }
        else
        {
            PRINT_DBG("Number of symmetric operation completed: %d\n",
                symStats.numSymOpCompleted);
        }
    }

    /* Clean up */

    /* Free session Context */
    OS_FREE(pSessionCtx);

    PRINT_DBG("cpaCyStopInstance\n");
    cpaCyStopInstance(CPA_INSTANCE_HANDLE_SINGLE);

    if (CPA_STATUS_SUCCESS == status)
    {
        PRINT_DBG("Sample code ran successfully\n");
    }
    else
    {
        PRINT_DBG("Sample code failed with status of %d\n", status);
    }
}
