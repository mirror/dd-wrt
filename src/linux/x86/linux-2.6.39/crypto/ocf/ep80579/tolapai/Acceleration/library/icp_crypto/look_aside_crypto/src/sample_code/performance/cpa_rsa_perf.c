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
 * @file cpaRsaPerf.c
 *
 * @defgroup sampleRsaPerfCode RSA 1024 bit Decrypt CRT Performance code
 *
 * @ingroup sampleCode
 *
 * @description
 *      This is a sample code that uses Diffie Hellman APIs.
 *       Functions contained in this file:
 *        - rsaDecryptCallback
 *        - rsaDecryptOpDataSetup
 *        - rsaFreeMemory
 *        - sampleRsaPerform
 * 
 *      This code pre-allocates a number of buffers as defined by
 *      NUM_PRE_ALLOCATED_BUFF_LISTS. The pre-allocated buffers are then
 *      continuously looped until NUM_RSA_DECRYPT_OPERATIONS is met.
 *      Time stamping is started prior to the first performed RSA Decrypt
 *      Operation and is stopped when all callbacks have returned.
 *      The code is called for each packet size as defined in cpaPerformance
 *
 *****************************************************************************/

#include "cpa.h"
#include "cpa_cy_common.h"
#include "cpa_cy_rsa.h"
#include "cpa_perf_defines.h"


/*============================================
 *    1024-bit RSA Key Pair parameters
 *==========================================*/

Cpa8U rsaPrimeP_1024_KeyPair[RSA_KEYPAIR_SIZE_IN_BYTES_64] =
{
    0xd3, 0x27, 0x37, 0xe7, 0x26, 0x7f, 0xfe, 0x13,
    0x41, 0xb2, 0xd5, 0xc0, 0xd1, 0x50, 0xa8, 0x1b, 
    0x58, 0x6f, 0xb3, 0x13, 0x2b, 0xed, 0x2f, 0x8d,
    0x52, 0x62, 0x86, 0x4a, 0x9c, 0xb9, 0xf3, 0x0a, 
    0xf3, 0x8b, 0xe4, 0x48, 0x59, 0x8d, 0x41, 0x3a,
    0x17, 0x2e, 0xfb, 0x80, 0x2c, 0x21, 0xac, 0xf1, 
    0xc1, 0x1c, 0x52, 0x0c, 0x2f, 0x26, 0xa4, 0x71,
    0xdc, 0xad, 0x21, 0x2e, 0xac, 0x7c, 0xa3, 0x9d
};

Cpa8U rsaPrimeQ_1024_KeyPair[RSA_KEYPAIR_SIZE_IN_BYTES_64] =
{
    0xcc, 0x88, 0x53, 0xd1, 0xd5, 0x4d, 0xa6, 0x30,
    0xfa, 0xc0, 0x04, 0xf4, 0x71, 0xf2, 0x81, 0xc7, 
    0xb8, 0x98, 0x2d, 0x82, 0x24, 0xa4, 0x90, 0xed,
    0xbe, 0xb3, 0x3d, 0x3e, 0x3d, 0x5c, 0xc9, 0x3c, 
    0x47, 0x65, 0x70, 0x3d, 0x1d, 0xd7, 0x91, 0x64,
    0x2f, 0x1f, 0x11, 0x6a, 0x0d, 0xd8, 0x52, 0xbe, 
    0x24, 0x19, 0xb2, 0xaf, 0x72, 0xbf, 0xe9, 0xa0,
    0x30, 0xe8, 0x60, 0xb0, 0x28, 0x8b, 0x5d, 0x77
};

Cpa8U rsaCrtExpDp_1024_KeyPair[RSA_KEYPAIR_SIZE_IN_BYTES_64] =
{
    0x0e, 0x12, 0xbf, 0x17, 0x18, 0xe9, 0xce, 0xf5,
    0x59, 0x9b, 0xa1, 0xc3, 0x88, 0x2f, 0xe8, 0x04, 
    0x6a, 0x90, 0x87, 0x4e, 0xef, 0xce, 0x8f, 0x2c,
    0xcc, 0x20, 0xe4, 0xf2, 0x74, 0x1f, 0xb0, 0xa3, 
    0x3a, 0x38, 0x48, 0xae, 0xc9, 0xc9, 0x30, 0x5f,
    0xbe, 0xcb, 0xd2, 0xd7, 0x68, 0x19, 0x96, 0x7d, 
    0x46, 0x71, 0xac, 0xc6, 0x43, 0x1e, 0x40, 0x37,
    0x96, 0x8d, 0xb3, 0x78, 0x78, 0xe6, 0x95, 0xc1
};

Cpa8U rsaCrtExpDq_1024_KeyPair[RSA_KEYPAIR_SIZE_IN_BYTES_64] =
{
    0x95, 0x29, 0x7b, 0x0f, 0x95, 0xa2, 0xfa, 0x67,
    0xd0, 0x07, 0x07, 0xd6, 0x09, 0xdf, 0xd4, 0xfc, 
    0x05, 0xc8, 0x9d, 0xaf, 0xc2, 0xef, 0x6d, 0x6e,
    0xa5, 0x5b, 0xec, 0x77, 0x1e, 0xa3, 0x33, 0x73, 
    0x4d, 0x92, 0x51, 0xe7, 0x90, 0x82, 0xec, 0xda,
    0x86, 0x6e, 0xfe, 0xf1, 0x3c, 0x45, 0x9e, 0x1a, 
    0x63, 0x13, 0x86, 0xb7, 0xe3, 0x54, 0xc8, 0x99,
    0xf5, 0xf1, 0x12, 0xca, 0x85, 0xd7, 0x15, 0x83
};

Cpa8U rsaCrtCoefQInv_1024_KeyPair[RSA_KEYPAIR_SIZE_IN_BYTES_64] =
{
    0x4f, 0x45, 0x6c, 0x50, 0x24, 0x93, 0xbd, 0xc0,
    0xed, 0x2a, 0xb7, 0x56, 0xa3, 0xa6, 0xed, 0x4d, 
    0x67, 0x35, 0x2a, 0x69, 0x7d, 0x42, 0x16, 0xe9,
    0x32, 0x12, 0xb1, 0x27, 0xa6, 0x3d, 0x54, 0x11, 
    0xce, 0x6f, 0xa9, 0x8d, 0x5d, 0xbe, 0xfd, 0x73,
    0x26, 0x3e, 0x37, 0x28, 0x14, 0x27, 0x43, 0x81, 
    0x81, 0x66, 0xed, 0x7d, 0xd6, 0x36, 0x87, 0xdd,
    0x2a, 0x8c, 0xa1, 0xd2, 0xf4, 0xfb, 0xd8, 0xe1
};

/*****************************************************************************/

void rsaDecryptCallback ( void *pCallbackTag,
    CpaStatus status,
    void *pOpdata,
    CpaFlatBuffer *pOut)
{
    perf_data_t *pPerfData = (perf_data_t *)pCallbackTag;

    if (pPerfData == NULL)
    {
        PRINT_ERR("Invalid data in CallbackTag\n");
        return;
    }

    /* response has been received */
    atomicInc(&pPerfData->responses);

    if (pPerfData->numOperations == atomicGet(&pPerfData->responses))
    {
        pPerfData->endCyclesTimestamp = CLOCK_TIMESTAMP;
        /*
         * once all callbacks are complete
         * finish timestamping global variables
         */
        COMPLETE(&pPerfData->comp);
    }
}

/*****************************************************************************/

static CpaStatus rsaDecryptOpDataSetup(
  CpaCyRsaDecryptOpData *pOpdata[],
  CpaFlatBuffer *pOutputData[] )
{
    CpaCyRsaDecryptOpData *pTmpOpdata     = NULL;
    CpaFlatBuffer         *pTmpOutputData = NULL;
    CpaCyRsaPrivateKey    *pTmpPrivateKey = NULL;

    CpaStatus status = CPA_STATUS_SUCCESS;
    Cpa32U    i = 0;

    for ( i = 0; i < NUM_PRE_ALLOCATED_BUFF_LISTS; i++)
    {
        if(CPA_STATUS_SUCCESS == status)
        {
            pTmpOutputData = (CpaFlatBuffer*)
                            allocOsMemCheck(sizeof(CpaFlatBuffer), &status);
        }

        if(CPA_STATUS_SUCCESS == status)
        {
            pTmpOutputData->pData = (Cpa8U*)allocOsMemCheck(
                                      RSA_KEYPAIR_SIZE_IN_BYTES_128, &status);

            pTmpOutputData->dataLenInBytes = RSA_KEYPAIR_SIZE_IN_BYTES_128;
        }

        pTmpOpdata = (CpaCyRsaDecryptOpData*)
                    allocOsMemCheck((sizeof(CpaCyRsaDecryptOpData)), &status);

        if( NULL == pTmpOpdata )
        {
            return CPA_STATUS_FAIL;
        }

        if(CPA_STATUS_SUCCESS == status)
        { 
            memset(pTmpOpdata, 0, sizeof(CpaCyRsaDecryptOpData)); 

            pTmpOpdata->inputData.dataLenInBytes =RSA_KEYPAIR_SIZE_IN_BYTES_128;

            pTmpOpdata->inputData.pData =
            (Cpa8U *)allocAlignedMem(RSA_KEYPAIR_SIZE_IN_BYTES_128,
                    BYTE_ALIGNMENT_64);

            if ( NULL == pTmpOpdata->inputData.pData )
            {
                return CPA_STATUS_FAIL;
            }

            if( CPA_STATUS_SUCCESS !=
                sampleRandGenPerform(pTmpOpdata->inputData.pData,
                    RSA_KEYPAIR_SIZE_IN_BYTES_128))
            {
                return CPA_STATUS_FAIL;
            }
        }

        /*********************/
        /* Setup Private Key */
        /*********************/
        if(CPA_STATUS_SUCCESS == status)
        {
            pTmpPrivateKey =
                       (CpaCyRsaPrivateKey *)allocAlignedMem(
                        sizeof(CpaCyRsaPrivateKey), BYTE_ALIGNMENT_64);

            if ( NULL == pTmpPrivateKey )
            {
                PRINT_ERR("No memory for pRecipientPrivateKey\n");
                return CPA_STATUS_FAIL;
            }

            /******************************/
            /* Setup version and key type */
            /******************************/
            pTmpPrivateKey->version = CPA_CY_RSA_VERSION_TWO_PRIME;
            pTmpPrivateKey->privateKeyRepType =
                                    CPA_CY_RSA_PRIVATE_KEY_REP_TYPE_2;

            /************************************/
            /* Copy in predefined RSA Key Pairs */
            /************************************/

            /***********/
            /* prime1P */
            /***********/
            pTmpPrivateKey->privateKeyRep2.prime1P.dataLenInBytes=
                RSA_KEYPAIR_SIZE_IN_BYTES_64;

            pTmpPrivateKey->privateKeyRep2.prime1P.pData =
                (Cpa8U *)allocAlignedMem(RSA_KEYPAIR_SIZE_IN_BYTES_64,
                        BYTE_ALIGNMENT_64); 


            if ( NULL == pTmpPrivateKey->privateKeyRep2.prime1P.pData )
            {
                PRINT_ERR("pRecipientPrivateKey PRIME1P me not allocated\n");
                return CPA_STATUS_FAIL;
            }

            memcpy( pTmpPrivateKey->privateKeyRep2.prime1P.pData,
                    &rsaPrimeP_1024_KeyPair[0], RSA_KEYPAIR_SIZE_IN_BYTES_64 );

            /***********/
            /* prime2Q */
            /***********/
            pTmpPrivateKey->privateKeyRep2.prime2Q.dataLenInBytes=
                RSA_KEYPAIR_SIZE_IN_BYTES_64;

            pTmpPrivateKey->privateKeyRep2.prime2Q.pData =
                (Cpa8U *)allocAlignedMem(RSA_KEYPAIR_SIZE_IN_BYTES_64,
                        BYTE_ALIGNMENT_64);


            if ( NULL == pTmpPrivateKey->privateKeyRep2.prime2Q.pData )
            {
                PRINT_ERR("pRecipientPrivateKey PRIME2Q mem not allocated\n");
                return CPA_STATUS_FAIL;
            }

            memcpy( pTmpPrivateKey->privateKeyRep2.prime2Q.pData,
                    &rsaPrimeQ_1024_KeyPair[0], RSA_KEYPAIR_SIZE_IN_BYTES_64 );

            /***************/
            /* exponent1Dp */
            /***************/
            pTmpPrivateKey->privateKeyRep2.exponent1Dp.dataLenInBytes =
                RSA_KEYPAIR_SIZE_IN_BYTES_64;

            pTmpPrivateKey->privateKeyRep2.exponent1Dp.pData = 
                (Cpa8U *)allocAlignedMem(RSA_KEYPAIR_SIZE_IN_BYTES_64,
                        BYTE_ALIGNMENT_64); 

            if( NULL == pTmpPrivateKey->privateKeyRep2.exponent1Dp.pData )
            {
                PRINT_ERR("exponent1Dp mem not allocated\n");
                return CPA_STATUS_FAIL;
            }

            memcpy( pTmpPrivateKey->privateKeyRep2.exponent1Dp.pData,
                  &rsaCrtExpDp_1024_KeyPair[0], RSA_KEYPAIR_SIZE_IN_BYTES_64 );

            /***************/
            /* exponent2Dq */
            /***************/
            pTmpPrivateKey->privateKeyRep2.exponent2Dq.dataLenInBytes =
                RSA_KEYPAIR_SIZE_IN_BYTES_64;

            pTmpPrivateKey->privateKeyRep2.exponent2Dq.pData =
                (Cpa8U *)allocAlignedMem(RSA_KEYPAIR_SIZE_IN_BYTES_64,
                        BYTE_ALIGNMENT_64);

            if( NULL == pTmpPrivateKey->privateKeyRep2.exponent2Dq.pData )
            {
                PRINT_ERR("exponent2Dq mem not allocated\n");
                return CPA_STATUS_FAIL;
            }

            memcpy( pTmpPrivateKey->privateKeyRep2.exponent2Dq.pData,
                    &rsaCrtExpDq_1024_KeyPair[0],
                    RSA_KEYPAIR_SIZE_IN_BYTES_64 );

            /**************/
            /* coeffQInv  */
            /**************/
            pTmpPrivateKey->privateKeyRep2.coefficientQInv.dataLenInBytes =
                RSA_KEYPAIR_SIZE_IN_BYTES_64;


            pTmpPrivateKey->privateKeyRep2.coefficientQInv.pData =
                (Cpa8U *)allocAlignedMem(RSA_KEYPAIR_SIZE_IN_BYTES_64,
                        BYTE_ALIGNMENT_64);

            if( NULL == pTmpPrivateKey->privateKeyRep2.coefficientQInv.pData )
            {
                PRINT_ERR("exponent2Dq mem not allocated\n");
                return CPA_STATUS_FAIL;
            }

            memcpy( pTmpPrivateKey->privateKeyRep2.coefficientQInv.pData,
                    &rsaCrtCoefQInv_1024_KeyPair[0],
                    RSA_KEYPAIR_SIZE_IN_BYTES_64);
        }

        pTmpOpdata->pRecipientPrivateKey = pTmpPrivateKey;

        if( CPA_STATUS_SUCCESS == status )
        {
            pOpdata[i] = pTmpOpdata;
            pOutputData[i] = pTmpOutputData;
        }
    }

    return status;
}

/*****************************************************************************/

static void rsaFreeMemory(CpaCyRsaDecryptOpData *pOpdata[],
                          CpaFlatBuffer *pOutputData[])
{
    Cpa32U   i = 0;
    CpaCyRsaPrivateKey *pTmpPrivateKey = NULL;

    for (i=0; i < NUM_PRE_ALLOCATED_BUFF_LISTS; i++)
    {
        if( NULL == (CpaCyRsaDecryptOpData *)pOpdata[i] )
        {
            return; 
        }

        if( NULL == (CpaFlatBuffer *)pOutputData[i] )
        {
            return; 
        }

        pTmpPrivateKey =
            ((CpaCyRsaDecryptOpData *)pOpdata[i])->pRecipientPrivateKey;

        OS_MEM_ALIGNED_FREE(
                pTmpPrivateKey->privateKeyRep2.coefficientQInv.pData);
        OS_MEM_ALIGNED_FREE(pTmpPrivateKey->privateKeyRep2.exponent2Dq.pData);
        OS_MEM_ALIGNED_FREE(pTmpPrivateKey->privateKeyRep2.exponent1Dp.pData);
        OS_MEM_ALIGNED_FREE(pTmpPrivateKey->privateKeyRep2.prime2Q.pData);
        OS_MEM_ALIGNED_FREE(pTmpPrivateKey->privateKeyRep2.prime1P.pData);

        OS_MEM_ALIGNED_FREE(
                ((CpaCyRsaDecryptOpData *)pOpdata[i])->pRecipientPrivateKey);


        OS_MEM_ALIGNED_FREE(
                ((CpaCyRsaDecryptOpData *)pOpdata[i])->inputData.pData);
        OS_MEM_FREE_CHECK((CpaCyRsaDecryptOpData *)pOpdata[i]);

        OS_MEM_FREE_CHECK(((CpaFlatBuffer *)pOutputData[i])->pData);
        OS_MEM_FREE_CHECK( (CpaFlatBuffer *)pOutputData[i]); 
    }

    return;
}

/******************************************************************************
 *  Main executing function
 *
 *****************************************************************************/

CpaStatus sampleRsaPerform (void)
{
    /* start of local variable declarations */
    CpaStatus status = CPA_STATUS_SUCCESS;

    /* RSA key parameters */
    Cpa32U primePLengthInBytes  = 0;
    Cpa32U primeQLengthInBytes  = 0;
    Cpa32U insideLoopCount      = 0;
    Cpa32U outsideLoopCount     = 0;
    perf_cycles_t numOfCycles   = 0;
    perf_cycles_t cyclesPerOp   = 0;
    perf_data_t *pRsaData       = NULL;
    Cpa32U  numOfLoops =
        (NUM_RSA_DECRYPT_OPERATIONS / NUM_PRE_ALLOCATED_BUFF_LISTS);

    /* Private key first representation parameters */
    CpaCyRsaDecryptOpData *pDecryptOpData[NUM_PRE_ALLOCATED_BUFF_LISTS];
    CpaFlatBuffer         *pOutputData[NUM_PRE_ALLOCATED_BUFF_LISTS];
    /* end of local varible declarations */

    /* Prime P and Q are always half the modulus size */
    primePLengthInBytes = primeQLengthInBytes = RSA_PRIME_P_Q_SIZE_IN_BYTES_64;

    pRsaData = (perf_data_t*)allocOsMemCheck(sizeof(perf_data_t), &status);

    if(CPA_STATUS_SUCCESS == status)
    {
        memset(pRsaData, 0, sizeof(perf_data_t));

        pRsaData->numOperations = NUM_RSA_DECRYPT_OPERATIONS;

        /* Completion used in callback */
        COMPLETION_INIT(&pRsaData->comp);

        status = rsaDecryptOpDataSetup(pDecryptOpData, pOutputData);
    }

    if(CPA_STATUS_SUCCESS == status)
    {
        /* Get the time and store in Global, collect this only for the first
         * request, the callback collects it for the last */
        pRsaData->startCyclesTimestamp = CLOCK_TIMESTAMP;

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
             * This inner for-loop loops around the number of Decrypt Op 
             * Data structs that have been pre-allocated.  Once the array
             * has completed- exit to outer loop and move to next iteration
             * of the pre-allocated data.
             */

            for(insideLoopCount = 0;
                insideLoopCount < NUM_PRE_ALLOCATED_BUFF_LISTS;
                insideLoopCount++)
            {

            /*
             * When the callback returns it will increment the responses
             * counter and test if its equal to 
             * NUM_RSA_DECRYPT_OPERATIONS, in that case all responses
             * have been successfully received.
             */
                
                do {
                       status = cpaCyRsaDecrypt(
                                CPA_INSTANCE_HANDLE_SINGLE,
                                rsaDecryptCallback,
                                pRsaData,
                                pDecryptOpData[insideLoopCount],
                                pOutputData[insideLoopCount] );

                        if(status == CPA_STATUS_RETRY)
                        {
                            atomicInc(&pRsaData->retries);

                            if(RETRY_LIMIT == atomicGet(&pRsaData->retries))
                            {
                                atomicSet(&pRsaData->retries, 0);

                                AVOID_SOFTLOCKUP;
                            }
                        }

                    }while (CPA_STATUS_RETRY == status);

                    if (CPA_STATUS_SUCCESS != status)
                    {
                        break;
                    }
            } /* end of inner loop */

            if (CPA_STATUS_SUCCESS != status)
            {
                break;
            }
        } /* end of outer loop */

    } /* end of if status success condition */

    if (CPA_STATUS_SUCCESS == status)
    {
        if(!COMPLETION_WAIT(&pRsaData->comp, OP_PERFORM_TIMEOUT))
        {
           PRINT_ERR("Timeout or interruption in cpaCySymPerformOp\n"); 
        }
    }

    if(CPA_STATUS_SUCCESS == status)
    {
        numOfCycles = (pRsaData->endCyclesTimestamp -
                       pRsaData->startCyclesTimestamp);

        PRINT_INFO("\n___________________________________________\n");
        PRINT_INFO("RSA CRT DECRYPT 1024 bit\n");
        PRINT_INFO("Total Responses\t%3u\n",
                    atomicGet(&pRsaData->responses));
        PRINT_INFO("Clock Cycles Start\t%12llu\n",
                    pRsaData->startCyclesTimestamp);
        PRINT_INFO("Clock Cycles End\t%12llu\n",
                    pRsaData->endCyclesTimestamp);
        PRINT_INFO("Total Cycles %14llu\n", numOfCycles);

        cyclesPerOp = getNumCyclesPerOp(numOfCycles,
                &pRsaData->responses, NUM_RSA_DECRYPT_OPERATIONS);

        PRINT_INFO("Cycles-Per-Operation %4llu\n", (perf_cycles_t)cyclesPerOp);
        PRINT_INFO("____________________________________________\n");
    }

    rsaFreeMemory(pDecryptOpData, pOutputData);

    COMPLETION_DESTROY(&pRsaData->comp);

    OS_MEM_FREE_CHECK(pRsaData);

    return status;
}
