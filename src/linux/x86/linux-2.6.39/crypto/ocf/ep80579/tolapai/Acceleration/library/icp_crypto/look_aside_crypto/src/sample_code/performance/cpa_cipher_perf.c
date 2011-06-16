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
 * @file cpa_cipher_perf.c
 *
 * @defgroup sampleCipherPerf Triple DES Cipher Block Chaining Performance code
 *
 * @ingroup perfCode 
 *
 * @description
 *      This file contains the main Triple DES CBC peformance sample code.
 *      Functions contained in this file:
 *        - cipherSetupSession
 *        - cipherPerformOpDataSetup
 *        - sampleCipherPerform
 * 
 *      This code pre-allocates a number of buffers as defined by
 *      NUM_PRE_ALLOCATED_BUFF_LISTS. The pre-allocated buffers are then
 *      continuously looped until the defined NUM_OPERATIONS is met.
 *      Time stamping is started prior to the first performed Triple DES CBC
 *      Operation and is stopped when all callbacks have returned.
 *      The code is called for each packet size as defined in cpaPerformance
 *
 *****************************************************************************/

#include "cpa.h"
#include "cpa_cy_sym.h"
#include "cpa_perf_defines.h"

/*****************************************************************************/

static CpaStatus cipherSetupSession( 
        CpaCySymCbFunc      pSymCb,
        Cpa8U               *pCipherKey, 
        CpaCySymSessionCtx  *pSession)
{
    CpaCySymSessionSetupData sessionData;
    Cpa32U                   pSessionCtxSizeInBytes = 0;
    CpaStatus                status                = CPA_STATUS_SUCCESS;
    CpaCySymSessionCtx       pLocalSession         = NULL;

    memset((void *) &sessionData, 0, sizeof(CpaCySymSessionSetupData));

    status = cpaCySymSessionCtxGetSize( CPA_INSTANCE_HANDLE_SINGLE,
            &sessionData, &pSessionCtxSizeInBytes);
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
        memset(pLocalSession, 0, pSessionCtxSizeInBytes);

        /* Setup all required elements of session data struct */

        sessionData.symOperation                    = CPA_CY_SYM_OP_CIPHER;
        sessionData.sessionPriority                 = CPA_CY_PRIORITY_HIGH;
        sessionData.cipherSetupData.cipherAlgorithm =
                                    CPA_CY_SYM_CIPHER_3DES_CBC;

        sessionData.cipherSetupData.cipherKeyLenInBytes =
            TDES_KEY_LENGTH_BYTES;
        sessionData.cipherSetupData.pCipherKey          = pCipherKey;
        sessionData.cipherSetupData.cipherDirection     =
            CPA_CY_SYM_CIPHER_DIRECTION_ENCRYPT;

        status = sampleRandGenPerform(sessionData.cipherSetupData.pCipherKey,
                    sessionData.cipherSetupData.cipherKeyLenInBytes);
        /*
         * initalize asynchronous callback- pLocalSession will contain
         * the session context 
         */
        if(CPA_STATUS_SUCCESS == status)
        {
            status = cpaCySymInitSession(CPA_INSTANCE_HANDLE_SINGLE, pSymCb,
                                &sessionData, pLocalSession);
        }

        if (CPA_STATUS_SUCCESS == status)
        {
            *pSession = pLocalSession;
        }
        else
        {
            PRINT_ERR("cipherSetupSession  Status = %d\n", status);
            OS_MEM_FREE_CHECK(pLocalSession);
        }
    }

    return status;
}

/*****************************************************************************/


static CpaStatus cipherPerformOpDataSetup(
        CpaCySymSessionCtx  pSessionCtx,
        Cpa32U              dataLenInBytes,  
        CpaCySymOpData      **pOpdata)
{
    CpaStatus      status      = CPA_STATUS_SUCCESS;
    CpaCySymOpData *pTmpOpdata = NULL;

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
        pTmpOpdata->ivLenInBytes                = TDES_CBC_IV_LENGTH_BYTES;

        pTmpOpdata->pIv = (Cpa8U *)allocAlignedMem(TDES_CBC_IV_LENGTH_BYTES,
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

/******************************************************************************
*  Main executing function
******************************************************************************/

CpaStatus sampleCipherPerform( Cpa32U packetSizeInBytes)
{
    /* start of local variable declarations */
    CpaCySymSessionCtx pSessionCtx  = NULL;
    CpaCySymOpData     *pOpData     = NULL;
    CpaFlatBuffer      *srcBuffPtrArray[NUM_PRE_ALLOCATED_BUFF_LISTS];
    CpaBufferList      *srcBuffListArray[NUM_PRE_ALLOCATED_BUFF_LISTS];
    perf_data_t        *pCipherData    = NULL;
    CpaStatus          status          = CPA_STATUS_SUCCESS;
    CpaBoolean         verifyResult    = CPA_FALSE;
    perf_cycles_t      numOfCycles      = 0;
    perf_cycles_t      cyclesPerOp      = 0;
    Cpa32U    numOfLoops  = (NUM_OPERATIONS / NUM_PRE_ALLOCATED_BUFF_LISTS);
    Cpa32U    outsideLoopCount = 0;
    Cpa32U    insideLoopCount  = 0;
    Cpa8U     key[TDES_KEY_LENGTH_BYTES];
    /* end of local varible declarations */

    pCipherData = (perf_data_t*)allocOsMemCheck(sizeof(perf_data_t), &status);
 

    if (CPA_STATUS_SUCCESS == status)
    {
        memset(pCipherData, 0, sizeof(perf_data_t));

        pCipherData->numOperations = NUM_OPERATIONS;

        /* Completion used in callback */
        COMPLETION_INIT(&pCipherData->comp);
    }


    if (CPA_STATUS_SUCCESS == status)
    {
        status = cipherSetupSession(performCallback, &key[0],
                &pSessionCtx);
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        status = cipherPerformOpDataSetup(pSessionCtx,
                packetSizeInBytes, &pOpData); 
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        status = sampleCreateBuffers( packetSizeInBytes,
                   srcBuffPtrArray, srcBuffListArray);
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        /* Get the time and store in Global, collect this only for the first
         * request, the callback collects it for the last */
        pCipherData->startCyclesTimestamp = CLOCK_TIMESTAMP;

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
                        pCipherData,
                        pOpData,
                        srcBuffListArray[insideLoopCount],
                        srcBuffListArray[insideLoopCount],
                        &verifyResult);

                    if(status == CPA_STATUS_RETRY)
                    {
                        atomicInc(&pCipherData->retries);

                        if(RETRY_LIMIT == atomicGet(&pCipherData->retries))
                        {
                            atomicSet(&pCipherData->retries, 0);

                            AVOID_SOFTLOCKUP;

                        }
                    }

                } while (CPA_STATUS_RETRY == status); 

                if (CPA_STATUS_SUCCESS != status)
                {
                    break;
                }

            } /*end of inner loop */

            if (CPA_STATUS_SUCCESS != status)
            {
                break;
            }
        } /* end of outer loop */
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        if(!COMPLETION_WAIT(&pCipherData->comp, OP_PERFORM_TIMEOUT))
        {
            PRINT_ERR("timeout or interruption in cpaCySymPerformOp\n");
        }
    }
        /* Free up resources allocated */
    if (CPA_STATUS_SUCCESS != cpaCySymRemoveSession(
                CPA_INSTANCE_HANDLE_SINGLE, pSessionCtx))
    {
        PRINT_ERR("Deregister session failed\n");
        status = CPA_STATUS_FAIL;
    }

    /* Print out performance figures */

    if (CPA_STATUS_SUCCESS == status)
    {
        numOfCycles = (pCipherData->endCyclesTimestamp -
                        pCipherData->startCyclesTimestamp);
        PRINT_INFO("\n_______________________________________\n");
        PRINT_INFO("Cipher Encrypt 3DES-CBC\n");
        PRINT_INFO("Total Responses\t%3u\n",
                atomicGet(&pCipherData->responses));
        PRINT_INFO("Packet Size %d\n", packetSizeInBytes);
        PRINT_INFO("Clock Cycles Start\t%12llu\n",
                pCipherData->startCyclesTimestamp);
        PRINT_INFO("Clock Cycles End\t%12llu\n",
                pCipherData->endCyclesTimestamp);
        PRINT_INFO("Total Cycles %14llu\n", numOfCycles);

        cyclesPerOp = getNumCyclesPerOp(numOfCycles, &pCipherData->responses,
                NUM_OPERATIONS);
        
        PRINT_INFO("Cycles-Per-Operation %4llu\n", (perf_cycles_t)cyclesPerOp);
        PRINT_INFO("_______________________________________\n");
    }

    sampleFreeBuffers( packetSizeInBytes, srcBuffPtrArray, srcBuffListArray);

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

    COMPLETION_DESTROY(&pCipherData->comp);

    OS_MEM_FREE_CHECK(pCipherData);
    OS_MEM_FREE_CHECK(pSessionCtx);
    OS_MEM_FREE_CHECK(pOpData);

    return status;
}
