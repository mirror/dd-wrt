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
 * @file lac_pke_utils.c
 *
 * @ingroup LacAsymComonUtils
 *
 * Implementation of utils that are PKE specific
 *
 ******************************************************************************/

/*
********************************************************************************
* Include public/global header files
********************************************************************************
*/
#include "cpa.h"
#include "icp_qat_fw_mmp_ids.h"

#include "lac_common.h"
#include "lac_mem.h"

#include "lac_pke_qat_comms.h"
#include "lac_pke_utils.h"

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

/**
*****************************************************************************
 * @ingroup LacAsymComonUtils
 *      Component state
 *
 * @description
 *      This enum is used to indicate the state that the component is in. This
 *     prevents PKE operations beign run if the component hasn't been
 *     initialised.
 *
 *****************************************************************************/
typedef enum {
    LAC_PKE_COMP_SHUT_DOWN = 0,
    /**< Component in the Shut Down state */
    LAC_PKE_COMP_INITIALISED,
    /**< Component in the initialised state */
}lac_pke_comp_state_t;

/*
 * Values below used to wait on completion of mmp liveness request
 * by polling completion variable.  10-second timeout chosen, to avoid
 * infinite wait
 */
#define LAC_PKE_WAIT_COUNT      (1000)
/**< Number of times to sleep and wakeup */
#define LAC_PKE_TIMEOUT_IN_MS   (10)
/**< Time to sleep in ms */

#define MMP_LIVENESS_DATA_SIZE 64

/**< Size of buffer to send dwon for PKE liveness requests */
#define PKE_FW_ID_IDX 0
/**< index of Cpa32U array where fw id can be found */

/* Points to a region of memory that has been zeroed out */
static Cpa8U lacPkeZeroBuf[LAC_BITS_TO_BYTES(LAC_MAX_OP_SIZE_IN_BITS)];
static CpaFlatBuffer lacPkeZeroFlatBuf;

/* State of Look Aside Crypto PKE */
/* @assumption - we're only supporting a single acceleration engine for now.
 * If supporting more than one, the pke init/shutdown state needs to be tracked
 * per engine.
 */
static IxOsalAtomic lacPkeState = IX_OSAL_ATOMIC_INIT(LAC_PKE_COMP_SHUT_DOWN);

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

Cpa32S
LacPke_Compare (
    const CpaFlatBuffer* pFlatBufferA,
    const Cpa32S deltaA,
    const CpaFlatBuffer* pFlatBufferB,
    const Cpa32S deltaB)
{
    Cpa32S diff = 0;
    Cpa32U length = 0;
    Cpa32U diffInLenBytes = 0;
    Cpa32S result = 0;
    CpaFlatBuffer bufA = {0};
    CpaFlatBuffer bufB = {0};

    LAC_ASSERT_NOT_NULL(pFlatBufferA);
    LAC_ASSERT_NOT_NULL(pFlatBufferB);
    LAC_ASSERT_NOT_NULL(pFlatBufferA->pData);
    LAC_ASSERT_NOT_NULL(pFlatBufferB->pData);

    LAC_ENSURE(((deltaA >= -128) && (deltaA <= 127)), "deltaA out of range");
    LAC_ENSURE(((deltaB >= -128) && (deltaB <= 127)), "deltaB out of range");
    
    /* Initially set the buf data members to the input buffers. These may change
     * if one buffer is larger than the other */
    bufA.pData = pFlatBufferA->pData;
    bufA.dataLenInBytes = pFlatBufferA->dataLenInBytes;

    bufB.pData = pFlatBufferB->pData;
    bufB.dataLenInBytes = pFlatBufferA->dataLenInBytes;

    /* If buffers are not equal in length then we need to compare the extra
     * bit of the longer buffer to zero. If this is non zero then that number
     * is greater.
     */
    if (pFlatBufferA->dataLenInBytes > pFlatBufferB->dataLenInBytes)
    {
        diffInLenBytes =
            (pFlatBufferA->dataLenInBytes - pFlatBufferB->dataLenInBytes);
        result = (Cpa32S)
            (memcmp(pFlatBufferA->pData, lacPkeZeroBuf, diffInLenBytes));
        if (0 != result)
        {
            /* A is larger so return positive value */
            return 1;
        }
        else
        {
            bufA.dataLenInBytes = pFlatBufferB->dataLenInBytes;
            bufA.pData = pFlatBufferA->pData + diffInLenBytes;
        }
    }
    else if (pFlatBufferB->dataLenInBytes > pFlatBufferA->dataLenInBytes)
    {
        diffInLenBytes =
            (pFlatBufferB->dataLenInBytes - pFlatBufferA->dataLenInBytes);
        result = (Cpa32S)
            (memcmp(pFlatBufferB->pData, lacPkeZeroBuf, diffInLenBytes));
        if (0 != result)
        {
            /* B is larger so return negative value */
            return -1;
        }
        else
        {
            bufB.dataLenInBytes = pFlatBufferA->dataLenInBytes;
            bufB.pData = pFlatBufferB->pData + diffInLenBytes;
        }
    }

    /* simulate a signed subtraction and return the sign of the result */
    /* We are dealing with Big Endian numbers so we work from int->pData[0]
     * to int->pData[length-1]. */
    while(length < bufA.dataLenInBytes - 1)
    {
        diff += bufA.pData[length];
        diff -= bufB.pData[length];
        if (0 != diff)
        {
            /* Numbers are different so return here */
            return diff;
        }
        length++;
    }
    diff += bufA.pData[length];
    diff -= bufB.pData[length];

    diff += deltaA - deltaB;

    return diff;
}

Cpa32S
LacPke_CompareZero (
    const CpaFlatBuffer* pLargeInteger,
    const Cpa32S delta)
{
    Cpa32S result = 0;
    LAC_ASSERT_NOT_NULL(pLargeInteger);
    if (0 == delta)
    {
        result = (Cpa32S) (memcmp(pLargeInteger->pData, lacPkeZeroBuf,
            pLargeInteger->dataLenInBytes));
    }
    else
    {   lacPkeZeroFlatBuf.dataLenInBytes = pLargeInteger->dataLenInBytes;
        result = LacPke_Compare(pLargeInteger, delta, &lacPkeZeroFlatBuf, 0);

    }
    return result;
}

CpaBoolean
LacPke_IsInitialised(void)
{
    if (LAC_PKE_COMP_INITIALISED == ixOsalAtomicGet(&lacPkeState))
    {
        return CPA_TRUE;
    }

    return CPA_FALSE;
}

STATIC void
LacPke_SetInitialised(lac_pke_comp_state_t state)
{
    ixOsalAtomicSet(state, &lacPkeState);
}

STATIC void
LacPke_VerifyMmpCbFunc(
    CpaStatus status,
    CpaBoolean pass,
    CpaInstanceHandle instanceHandle,
    lac_pke_op_cb_data_t *pCbData)
{
    CpaBoolean *pCompletion = NULL;

    LAC_ASSERT_NOT_NULL(pCbData);
    pCompletion = (CpaBoolean *)pCbData->pOpaqueData;

    if ((CPA_STATUS_SUCCESS == status ) && (NULL != pCompletion))
    {
        /* set the completion to true to unblock function pending on this */
        *pCompletion = CPA_TRUE;
    }
    else
    {
        LAC_LOG_ERROR("Error in internal callback for PKE MMP Liveness");
    }
}

STATIC CpaStatus
LacPke_VerifyMmpLib(void)
{
    CpaStatus status = CPA_STATUS_FAIL;
    icp_qat_fw_mmp_input_param_t inArgList;
    icp_qat_fw_mmp_output_param_t outArgList;
    Cpa32U inArgSizeList[LAC_MAX_MMP_INPUT_PARAMS] = {0};
    Cpa32U outArgSizeList[LAC_MAX_MMP_OUTPUT_PARAMS] = {0};
    lac_pke_op_cb_data_t cbData;
    CpaFlatBuffer outBuffer = {0};
    Cpa32U *pOutBufferData = NULL;
    volatile CpaBoolean *pCompletion = NULL;
    Cpa32U count = 0;
    Cpa32U loadedFwId = 0;
    Cpa32U swappedFwId = 0;
    outArgSizeList[0] = MMP_LIVENESS_DATA_SIZE;

    status = LAC_OS_MALLOC(
                LAC_CONST_VOLATILE_PTR_CAST(&pCompletion), 
                sizeof(CpaBoolean));

    if (CPA_STATUS_SUCCESS == status)
    {
        status = LAC_OS_MALLOC(&pOutBufferData, MMP_LIVENESS_DATA_SIZE);
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        LAC_OS_BZERO(&inArgList, sizeof(inArgList));
        LAC_OS_BZERO(&outArgList, sizeof(outArgList));
        LAC_OS_BZERO(pOutBufferData, MMP_LIVENESS_DATA_SIZE);

        outBuffer.dataLenInBytes = MMP_LIVENESS_DATA_SIZE;
        outBuffer.pData = (Cpa8U *)pOutBufferData;

        LAC_MEM_SHARED_WRITE_FROM_PTR(outArgList.flat_array[0], &outBuffer);

        /* set up while check variable */
        *pCompletion = CPA_FALSE;
        cbData.pOpaqueData = LAC_CONST_PTR_CAST(pCompletion);

        status = LacPke_SendSingleRequest(
            PKE_LIVENESS,
            inArgSizeList,
            outArgSizeList,
            &inArgList,
            &outArgList,
            LacPke_VerifyMmpCbFunc,
            &cbData,
            0);
    }

    /* pend until the lock is unlocked by callback function */
    if (CPA_STATUS_SUCCESS == status)
    {
        while ((CPA_FALSE == *pCompletion) && (count < LAC_PKE_WAIT_COUNT))
        {
            ixOsalSleep(LAC_PKE_TIMEOUT_IN_MS);
            count ++;
        }

        if (CPA_TRUE == *pCompletion)
        {
            void *ptr = LAC_CONST_VOLATILE_PTR_CAST(pCompletion);
            LAC_OS_FREE(ptr);
            pCompletion = ptr; /*volatiling again*/
            loadedFwId = pOutBufferData[PKE_FW_ID_IDX];
            LAC_OS_FREE(pOutBufferData);
        }
        else
        {
            /* The completion is not freed when then the timeout expires.
             * This results in a memory leak but it will prevent a system
             * crash. The callback may arrive at some stage in the future and
             * the callback function will try and access the memory for the
             * completion to set it to true. Therefore the memory must for the
             * completion must remain */
            LAC_LOG_ERROR("Timeout for MMP Liveness callback has expired");
            status = CPA_STATUS_FAIL;
        }
    }
    else
    {
        void *ptr = LAC_CONST_VOLATILE_PTR_CAST(pCompletion); /*de-volatile*/
        LAC_OS_FREE(ptr);
        pCompletion = ptr; /*volatile again*/
        LAC_OS_FREE(pOutBufferData);
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        LAC_MEM_SHARED_READ(loadedFwId, swappedFwId);

        if (swappedFwId != PKE_INTERFACE_SIGNATURE)
        {
            LAC_LOG_ERROR2("Error in LAC initialisation. Compiled firmware " 
                "version (0x%08X) does not match loaded firmware version " 
                "(0x%08X)",PKE_INTERFACE_SIGNATURE, swappedFwId);
            status = CPA_STATUS_FAIL;
        }
    }

    return status;
}

void
LacPke_Shutdown(void)
{
    LacPke_SetInitialised(LAC_PKE_COMP_SHUT_DOWN);
}

CpaStatus
LacPke_Init(void)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    lacPkeZeroFlatBuf.dataLenInBytes
        = LAC_BITS_TO_BYTES(LAC_MAX_OP_SIZE_IN_BITS);
    LAC_OS_BZERO(lacPkeZeroBuf, lacPkeZeroFlatBuf.dataLenInBytes);
        lacPkeZeroFlatBuf.pData = lacPkeZeroBuf;

    status = LacPke_VerifyMmpLib();

    if (CPA_STATUS_SUCCESS == status)
    {
        /* change state to initialised */
        LacPke_SetInitialised(LAC_PKE_COMP_INITIALISED);
    }
    else
    {
        LAC_LOG_ERROR("Error in LAC initialisation. Cannot initialise PKE");
        
        /* change state to unitialised */
        LacPke_SetInitialised(LAC_PKE_COMP_SHUT_DOWN);
    }

    return status;
}
