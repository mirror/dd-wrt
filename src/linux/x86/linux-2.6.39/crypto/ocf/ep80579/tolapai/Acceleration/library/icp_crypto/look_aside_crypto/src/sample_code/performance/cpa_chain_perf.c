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

/*
 *****************************************************************************
 * Doxygen group definitions
 ****************************************************************************/

/**
 *****************************************************************************
 * @file cpa_chain_perf.c
 *
 * @defgroup sampleChainPerf Algorithm chaining 3DES-CBC, HMAC-SHA1
 *
 * @ingroup perfCode
 *
 * @description
 *      This file contains the main function required for performing
 *      algorithm chaining operations.
 *      There are 2 key steps:
 *        - perform Triple DES-CBC operation
 *        - perform HMAC-SHA1 operation
 * 
 *      Functions contained in this file:
 *        - chainSetupSession
 *        - chainPerformOpDataSetup
 *        - sampleAlgoChainPerform
 *
 *****************************************************************************/


#include "cpa.h"
#include "cpa_cy_common.h"
#include "cpa_cy_sym.h"
#include "cpa_perf_defines.h"


/*****************************************************************************/

/* register a cipher session */
static CpaStatus chainSetupSession( 
        CpaCySymCbFunc      pSymCb,
        Cpa8U               *pCipherKey,
        Cpa8U               *pAuthKey,
        CpaCySymSessionCtx  *pSession)
{
    CpaCySymSessionSetupData  sessionData;
    Cpa32U                    pSessionCtxSizeInBytes = 0;
    CpaStatus                 status                 = CPA_STATUS_SUCCESS;
    CpaCySymSessionCtx        pLocalSession          = NULL;

    memset((void *) &sessionData, 0, sizeof(CpaCySymSessionSetupData));

    status = cpaCySymSessionCtxGetSize( CPA_INSTANCE_HANDLE_SINGLE,
            &sessionData,  &pSessionCtxSizeInBytes);

    /*
     * Setup local Session Context 
     */

    if( CPA_STATUS_SUCCESS == status)
    {
        pLocalSession = (CpaCySymSessionCtx)
                          allocOsMemCheck(pSessionCtxSizeInBytes, &status);
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        memset(pLocalSession, 0 , pSessionCtxSizeInBytes);

        sessionData.symOperation = CPA_CY_SYM_ALG_CHAIN_ORDER_CIPHER_THEN_HASH;
        sessionData.sessionPriority = CPA_CY_PRIORITY_HIGH;

        sessionData.cipherSetupData.cipherAlgorithm     =
                                            CPA_CY_SYM_CIPHER_3DES_CBC;
        sessionData.cipherSetupData.pCipherKey          = 
                                            pCipherKey;
        sessionData.cipherSetupData.cipherKeyLenInBytes =
                                            TDES_KEY_LENGTH_BYTES;
        sessionData.cipherSetupData.cipherDirection     =
                                            CPA_CY_SYM_CIPHER_DIRECTION_ENCRYPT;

        status = sampleRandGenPerform(sessionData.cipherSetupData.pCipherKey,
            sessionData.cipherSetupData.cipherKeyLenInBytes);

        sessionData.hashSetupData.hashAlgorithm     = CPA_CY_SYM_HASH_SHA1;
        sessionData.hashSetupData.hashMode          =
                                            CPA_CY_SYM_HASH_MODE_AUTH;
        sessionData.hashSetupData.digestResultLenInBytes = SHA1_DIGEST_LENGTH; 

        sessionData.hashSetupData.authModeSetupData.authKey = pAuthKey;
        sessionData.hashSetupData.authModeSetupData.authKeyLenInBytes =
                                            SHA1_KEY_LENGTH;

        status = sampleRandGenPerform(
                sessionData.hashSetupData.authModeSetupData.authKey,
                sessionData.hashSetupData.authModeSetupData.authKeyLenInBytes);

        if(CPA_STATUS_SUCCESS == status)
        {
            status = cpaCySymInitSession(CPA_INSTANCE_HANDLE_SINGLE,
                    pSymCb, &sessionData, pLocalSession);
        }

        if (CPA_STATUS_SUCCESS == status)
        {
            *pSession = pLocalSession;
        }
        else
        {
            PRINT_ERR("chainSetupSession()  Status = %d\n", status);
            /* if pLocalSession is not null call kfree */
            OS_MEM_FREE_CHECK(pLocalSession);
        }
    }

    return status;
}

/*****************************************************************************/

static CpaStatus chainPerformOpDataSetup(
        CpaCySymSessionCtx  pSessionCtx,
        Cpa32U              dataLenInBytes,
        CpaCySymOpData      **pOpdata,
        CpaFlatBuffer       *pFlatBuffArray[])
{
    CpaStatus        status = CPA_STATUS_SUCCESS;
    CpaCySymOpData   *pTmpOpdata     = NULL;

    pTmpOpdata = (CpaCySymOpData*)
                     allocOsMemCheck(sizeof(CpaCySymOpData), &status);

    if ( NULL == pTmpOpdata )
    {
        status = CPA_STATUS_FAIL;
    }

    if(CPA_STATUS_SUCCESS == status)
    {
        memset(pTmpOpdata, 0, sizeof(CpaCySymOpData)); 

        pTmpOpdata->pSessionCtx                 = pSessionCtx;
        pTmpOpdata->packetType                  = CPA_CY_SYM_PACKET_TYPE_FULL;
        pTmpOpdata->cryptoStartSrcOffsetInBytes = CIPHER_OFFSET_BYTES;
        pTmpOpdata->messageLenToCipherInBytes   = dataLenInBytes;
        pTmpOpdata->hashStartSrcOffsetInBytes   = HASH_OFFSET_BYTES;
        pTmpOpdata->messageLenToHashInBytes     = dataLenInBytes;  
        pTmpOpdata->ivLenInBytes  = TDES_CBC_IV_LENGTH_BYTES;
        pTmpOpdata->digestVerify  = CPA_FALSE;
        pTmpOpdata->pDigestResult =
            (pFlatBuffArray[0]->pData + dataLenInBytes);

        pTmpOpdata->pIv = (uint8_t*)allocAlignedMem(TDES_CBC_IV_LENGTH_BYTES,
            BYTE_ALIGNMENT_8);

        if ( NULL == pTmpOpdata->pIv )
        {
            PRINT_ERR("IV is null\n");
            status = CPA_STATUS_FAIL;
        }
    }

    if(CPA_STATUS_SUCCESS == status)
    {
        status = sampleRandGenPerform(pTmpOpdata->pIv,
                TDES_CBC_IV_LENGTH_BYTES);
    }

    if(CPA_STATUS_SUCCESS == status)
    {
        *pOpdata = pTmpOpdata;
    }

    return status;
}

/*****************************************************************************
*  Main exectuting function
*****************************************************************************/

CpaStatus sampleAlgoChainPerform( Cpa32U packetSizeInBytes)
{
    /* start of local variable declarations */
    CpaFlatBuffer       *srcBuffPtrArray[NUM_PRE_ALLOCATED_BUFF_LISTS];
    CpaBufferList       *srcBuffListArray[NUM_PRE_ALLOCATED_BUFF_LISTS];
    CpaCySymSessionCtx  pSessionCtx = NULL;
    CpaCySymOpData      *pOpData    = NULL;
    perf_data_t         *pChainData = NULL;
    CpaStatus           status      = CPA_STATUS_SUCCESS;
    CpaBoolean  verifyResult        = CPA_FALSE;
    Cpa32U      hashTotalSizeInBytes =
        (packetSizeInBytes + SHA1_DIGEST_LENGTH);
    Cpa32U      numOfLoops =
        (NUM_OPERATIONS / NUM_PRE_ALLOCATED_BUFF_LISTS);
    Cpa32U      outsideLoopCount = 0;
    Cpa32U      insideLoopCount  = 0;
    Cpa8U       cipherKey[TDES_KEY_LENGTH_BYTES];
    Cpa8U       authKey[AUTH_KEY_LENGTH_64];  
    perf_cycles_t    numOfCycles      = 0;
    perf_cycles_t    cyclesPerOp      = 0;
    /* end of local varible declarations */

    pChainData = (perf_data_t*)allocOsMemCheck(sizeof(perf_data_t), &status);

    if (CPA_STATUS_SUCCESS == status)
    {
        memset(pChainData, 0, sizeof(perf_data_t));

        pChainData->numOperations = NUM_OPERATIONS;

        /* Completion used in callback */
        COMPLETION_INIT(&pChainData->comp);
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        status = chainSetupSession(performCallback,
            &cipherKey[0], &authKey[0], &pSessionCtx);
    }

    /*
     * Call sampleCreateBuffers to allocate memory for all
     * the pre-allocated Buffer Lists and Flat Buffers.
     */

    if (CPA_STATUS_SUCCESS == status)
    {
        status = sampleCreateBuffers( hashTotalSizeInBytes,
                srcBuffPtrArray, srcBuffListArray);
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        status = chainPerformOpDataSetup(pSessionCtx,
                packetSizeInBytes, &pOpData, srcBuffPtrArray);
    }

    if(CPA_STATUS_SUCCESS == status)
    {
        /*
         * Get the number of clock cycles and store in global
         * variable.  once the final callabck is received
         * the number of cycles is again collected and this
         * represents the time it took to perform all operations
         */
        pChainData->startCyclesTimestamp = (perf_cycles_t)CLOCK_TIMESTAMP;

        /* 
         * The outside for-loop will loop around the pre-allocated buffer list
         * array the number of times necessary to satisfy:
         * NUM_OPERATIONS / NUM_PRE_ALLOCATED_BUFF_LISTS
         */

        for( outsideLoopCount = 0;
             outsideLoopCount < numOfLoops;
             outsideLoopCount++)
        {
            /* 
             * This inner for-loop loops around the number of Buffer Lists
             * that have been pre-allocated.  Once the array has completed-
             * exit to the outer loop to move on the next iteration of the
             * pre-allocated loop.
             */

            for (insideLoopCount = 0;
                 insideLoopCount < NUM_PRE_ALLOCATED_BUFF_LISTS;
                 insideLoopCount++)
            {
               /*
                 * When the callback returns it will increment the responses
                 * counter and test if its equal to NUM_OPERATIONS, in that
                * case all responses have been successfully received.
                 */

                do {
                    status = cpaCySymPerformOp (
                            CPA_INSTANCE_HANDLE_SINGLE,
                            pChainData, 
                            pOpData, 
                            srcBuffListArray[insideLoopCount], 
                            srcBuffListArray[insideLoopCount], 
                            &verifyResult);

                    if(status == CPA_STATUS_RETRY)
                    {
                        atomicInc(&pChainData->retries);

                        if(RETRY_LIMIT == atomicGet(&pChainData->retries))
                        {
                            atomicSet(&pChainData->retries, 0);

                            AVOID_SOFTLOCKUP;
                        }
                    }
                } while (CPA_STATUS_RETRY == status); 

                if (CPA_STATUS_FAIL == status)
                {
                    break;
                }

            } /*end of inner loop */

            if (CPA_STATUS_SUCCESS != status)
            {
                break;
            }
        } /* end of outer loop */

    } /* end if status success */

    if (CPA_STATUS_SUCCESS == status)
    {
        if(!COMPLETION_WAIT(&pChainData->comp, OP_PERFORM_TIMEOUT))
        {
            PRINT_ERR("timeout or interruption in cpaCySymPerformOp\n");
        }

    }

        /* Free up resources allocated */
    if (CPA_STATUS_SUCCESS != cpaCySymRemoveSession(CPA_INSTANCE_HANDLE_SINGLE,
                pSessionCtx))
    {
        PRINT_ERR("Deregister session failed\n");
        status = CPA_STATUS_FAIL;
    }

    /* Print out performance figures */

    if (CPA_STATUS_SUCCESS == status)
    {
        numOfCycles = (pChainData->endCyclesTimestamp -
                        pChainData->startCyclesTimestamp);
        PRINT_INFO("\n___________________________________________\n");
        PRINT_INFO("Algorithm Chaining - 3DES-CBC HMAC-SHA1\n");
        PRINT_INFO("Total Responses\t%3u\n",
                atomicGet(&pChainData->responses));
        PRINT_INFO("Packet Size %d\n", packetSizeInBytes);
        PRINT_INFO("Clock Cycles Start\t%12llu\n",
                pChainData->startCyclesTimestamp);
        PRINT_INFO("Clock Cycles End\t%12llu\n",
                pChainData->endCyclesTimestamp);
        PRINT_INFO("Total Cycles %14llu\n", numOfCycles);

        cyclesPerOp = getNumCyclesPerOp(numOfCycles,
                &pChainData->responses, NUM_OPERATIONS);

        PRINT_INFO("Cycles-Per-Operation %4llu\n", (perf_cycles_t)cyclesPerOp);
        PRINT_INFO("___________________________________________\n");
    }

    sampleFreeBuffers(hashTotalSizeInBytes,srcBuffPtrArray, srcBuffListArray);

    if ( NULL == pOpData )
    {
        PRINT_ERR("pOpData is NULL\n");
        return CPA_STATUS_FAIL;
    }

    if ( NULL == pOpData->pIv )
    {
        PRINT_ERR("IV is null\n");
        status = CPA_STATUS_FAIL;
    }
    else
    {
        OS_MEM_ALIGNED_FREE((void *)pOpData->pIv);
    }

    COMPLETION_DESTROY(&pChainData->comp);

    OS_MEM_FREE_CHECK(pChainData);
    OS_MEM_FREE_CHECK(pSessionCtx);
    OS_MEM_FREE_CHECK(pOpData);

    return status;
}
