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
 * cpa_cipher_sample.c
 * 
 * sampleCipherFunctional
 *
 *  sampleCode
 *  
 * This is sample code that uses Cipher APIs. It ciphers a sample text with 
 * triple DES algorithm.
 * 
 *****************************************************************************/

#include "cpa.h"
#include "cpa_cy_im.h"
#include "cpa_cy_sym.h"

/** Definition of macros for memory allocation and print*/
#include "../../include/cpa_sample_utils.h"

#define TIMEOUT_MS 5000 /* 5 seconds*/

extern int debugParam ;

static Cpa8U sampleCipherKey[] = {
        0xEE,0xE2,0x7B,0x5B,0x10,0xFD,0xD2,0x58,0x49,0x77,0xF1,0x22,
        0xD7,0x1B,0xA4,0xCA,0xEC,0xBD,0x15,0xE2,0x52,0x6A,0x21,0x0B
};
/** sampleCipherFunctional
 * Triple DES key: 192 bits long*/

static Cpa8U sampleCipherIv[] = {
        0x7E,0x9B,0x4C,0x1D,0x82,0x4A,0xC5,0xDF
};
/** sampleCipherFunctional
 *  Initialization vector*/

static Cpa8U sampleCipherSrc[] = {
        0xD7,0x1B,0xA4,0xCA,0xEC,0xBD,0x15,0xE2,0x52,0x6A,0x21,0x0B,
        0x81,0x77,0x0C,0x90,0x68,0xF6,0x86,0x50,0xC6,0x2C,0x6E,0xED,
        0x2F,0x68,0x39,0x71,0x75,0x1D,0x94,0xF9,0x0B,0x21,0x39,0x06,
        0xBE,0x20,0x94,0xC3,0x43,0x4F,0x92,0xC9,0x07,0xAA,0xFE,0x7F,
        0xCF,0x05,0x28,0x6B,0x82,0xC4,0xD7,0x5E,0xF3,0xC7,0x74,0x68,
        0xCF,0x05,0x28,0x6B,0x82,0xC4,0xD7,0x5E,0xF3,0xC7,0x74,0x68,
        0x80,0x8B,0x28,0x8D,0xCD,0xCA,0x94,0xB8,0xF5,0x66,0x0C,0x00,
        0x5C,0x69,0xFC,0xE8,0x7F,0x0D,0x81,0x97,0x48,0xC3,0x6D,0x24
};
/** sampleCipherFunctional
 *  Source data to encrypt*/

static Cpa8U expectedOutput[] = {
        0x35,0x0C,0x46,0xF8,0xFE,0x13,0x8A,0x7C,0x9B,0x66,0x83,0x5F,
        0x94,0xDC,0x4F,0x96,0x66,0x56,0x35,0xC3,0xFA,0xFD,0x51,0xA1,
        0xC9,0x3B,0xAF,0x06,0x2A,0xA9,0x54,0x0D,0xF1,0x0B,0xBB,0xB1,
        0x27,0x15,0x9D,0xD2,0x08,0xAC,0xF0,0x92,0x47,0x19,0xE2,0xC1,
        0x47,0xAC,0x34,0x30,0x8C,0x95,0x1B,0x14,0xD4,0x71,0x37,0x4B,
        0x50,0xCB,0x73,0xAA,0x4F,0x98,0x36,0xF1,0x97,0xE2,0x8C,0x37,
        0x6C,0x44,0xC2,0xFD,0xAD,0xE4,0xF5,0x56,0x62,0x92,0xEF,0x84,
        0x9E,0x33,0x0D,0x5B,0x34,0x27,0xA0,0x2B,0x9B,0x7C,0xE7,0x8A,
};
/** sampleCipherFunctional
 *  Expected Output */

/******************************************************************************
 * sampleCipherFunctional
 ****************************************************************************/
void
cipherSample(void);

/**
 *****************************************************************************
 *  sampleCipherFunctional
 * Symmetric callback function
 * It is a signal that the operation is completed. The user can call the
 * application above, free the memory, etc.
 * In this example, the function only sets the complete variable to indicate
 * it has been called
 *
 ****************************************************************************/
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
 *  sampleCipherFunctional
 * Perform an cipher operation
 *
 ****************************************************************************/
static CpaStatus 
cipherPerformOp(CpaCySymSessionCtx pSessionCtx)
{
    CpaStatus       status          = CPA_STATUS_SUCCESS;
    Cpa8U           *pBufferMeta    = NULL;
    Cpa8U           *pSrcBuffer     = NULL;
    Cpa8U           *pIvBuffer      = NULL;
    Cpa32U          bufferMetaSize  = 0;
    CpaBufferList   *pBufferList    = NULL;
    CpaFlatBuffer   *pFlatBuffer    = NULL;
    CpaCySymOpData  *pOpData        = NULL; 
    Cpa32U          bufferSize      = sizeof(sampleCipherSrc); 
    Cpa32U          numBuffers      = 1;  /* only using 1 buffer in this case */
    Cpa32U      bufferListMemSize   = sizeof(CpaBufferList) + 
                                        (numBuffers * sizeof(CpaFlatBuffer));

    /* allocate memory for bufferlist and array of flat buffers in a contiguous 
     * area and carve it up to reduce number of memory allocations required. */

    /* The following variables are allocated on the stack because we block
     * until the callback comes back. If a non-blocking approach was to be
     * used then these variables should be dynamically allocated */
    struct COMPLETION_STRUCT complete;
    COMPLETION_INIT(&complete);
    /** initialisation for callback; the "complete" variable is used by the
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
        status = OS_MALLOC(&pIvBuffer, sizeof(sampleCipherIv));
    }

    if (CPA_STATUS_SUCCESS == status) 
    {
        /* copy source into buffer */
        memcpy(pSrcBuffer, sampleCipherSrc, sizeof(sampleCipherSrc));

        /* copy IV into buffer */
        memcpy(pIvBuffer, sampleCipherIv, sizeof(sampleCipherIv));

        
        pBufferList->pBuffers = pFlatBuffer;
        pBufferList->numBuffers = 1;
        pBufferList->pPrivateMetaData = pBufferMeta;
    
        pFlatBuffer->dataLenInBytes = bufferSize;
        pFlatBuffer->pData = pSrcBuffer;     
        
        status = OS_MALLOC(&pOpData, sizeof(CpaCySymOpData));
    }

    if (CPA_STATUS_SUCCESS == status) 
    {
        /** Populate the structure containing the operational data that is
         * needed to run the algorithm:
         * - packet type information (the algorithm can operate on a full 
         * packet, perform a partial operation and maintain the state or 
         * complete the last part of a multi-part operation)
         * - the initialization vector and its length
         * - the offset in the source buffer
         * - the length of the source message
         */         
        pOpData->pSessionCtx = pSessionCtx;
        pOpData->packetType = CPA_CY_SYM_PACKET_TYPE_FULL;
        pOpData->pIv = pIvBuffer;
        pOpData->ivLenInBytes = sizeof(sampleCipherIv);
        pOpData->cryptoStartSrcOffsetInBytes = 0;
        pOpData->messageLenToCipherInBytes = sizeof(sampleCipherSrc);
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
            if (!COMPLETION_WAIT(&complete, TIMEOUT_MS))
            {
                PRINT_ERR("timeout or interruption in cpaCySymPerformOp\n");
                status = CPA_STATUS_FAIL;
            }
        }

        if (CPA_STATUS_SUCCESS == status)
        {
            if (0 == memcmp(pSrcBuffer, expectedOutput, bufferSize))
            {
                PRINT_DBG("Output matches expected output\n");
            }
            else
            {
                PRINT_DBG("Output does not match expected output\n");
                status = CPA_STATUS_FAIL;
            }
        }
    }

    /* The callback function has returned, so it is sure that
     * the structures won't be needed any more*/
    OS_FREE(pSrcBuffer);
    OS_FREE(pIvBuffer);
    OS_FREE(pBufferList);
    OS_FREE(pBufferMeta);
    OS_FREE(pOpData);

    COMPLETION_DESTROY(&complete);

    return status;
}

void
cipherSample(void)
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
        /* Populate the session setup structure for the operation required */
        sessionSetupData.sessionPriority =  CPA_CY_PRIORITY_NORMAL;
        sessionSetupData.symOperation =     CPA_CY_SYM_OP_CIPHER;
        sessionSetupData.cipherSetupData.cipherAlgorithm =
                                            CPA_CY_SYM_CIPHER_3DES_CBC;
        sessionSetupData.cipherSetupData.pCipherKey = 
                                            sampleCipherKey;
        sessionSetupData.cipherSetupData.cipherKeyLenInBytes = 
                                            sizeof(sampleCipherKey);
        sessionSetupData.cipherSetupData.cipherDirection = 
                                            CPA_CY_SYM_CIPHER_DIRECTION_ENCRYPT;

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


    /** Initialize the Cipher session */
    if (CPA_STATUS_SUCCESS == status) {
        PRINT_DBG("cpaCySymInitSession\n");
        status = cpaCySymInitSession(CPA_INSTANCE_HANDLE_SINGLE,
                            symCallback,        /* callback function */
                            &sessionSetupData,  /* session setup data */
                            pSessionCtx);       /* output of the function*/
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        CpaStatus sessionStatus = CPA_STATUS_SUCCESS;
 
        /* Perform Cipher operation */
        status = cipherPerformOp(pSessionCtx);

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
