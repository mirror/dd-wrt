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
 * @file cpa_main_utils_perf.c
 *
 * @defgroup sampleUtilsPerf  Main and Utility functions for Performance code 
 *
 * @ingroup perfCode
 *
 * @description
 *      This file contains utility functions for peformance sample code  
 *
 *****************************************************************************/

/******************************************************************************
 * 
 *
 *      Performance code is run in the following order:
 *        - Cipher (3DES-CBC) 
 *        - Hash  (HMAC-SHA1)
 *        - Algo-Chaining (3DES-CBC, HMAC-SHA1) 
 *        - RSA-Decrypt-CRT 
 * 
 *      Functions contained in this file:
 *        - atomicGet
 *        - atomicInc
 *        - atomicSet
 *        - getNumCyclesPerOp
 *        - performCallback
 *        - sampleRandGenPerform
 *        - samplePerformance
 *
 *****************************************************************************/

#if defined(__linux)
    #include <linux/io.h>
#endif

#include "cpa.h"
#include "cpa_cy_common.h"
#include "cpa_cy_sym.h"
#include "cpa_cy_rand.h"
#include "cpa_cy_im.h"
#include "cpa_perf_defines.h"


/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      This function acts as a wrapper for reading the value of an atomic
 *      variable
 *
 *****************************************************************************/

Cpa32U
atomicGet(perf_atomic_t *atomicVar)
{
#if defined(__linux)
    return ((Cpa32U)atomic_read(atomicVar));
#elif defined(__freebsd)   
    return ((Cpa32U)atomic_load_acq_int(atomicVar));
#endif

}

/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      This function acts as a wrapper for incrementing the value of an 
 *      atomic variable by 1
 *
 *****************************************************************************/

void
atomicInc(perf_atomic_t *atomicVar)
{
#if defined(__linux)
    atomic_inc(atomicVar);
#elif defined(__freebsd)
    atomic_add_rel_int(atomicVar, 1);
#endif
}

/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      This function acts as a wrapper to set the value of an atomic
 *      variable to the inValue
 *
 *****************************************************************************/

void
atomicSet(perf_atomic_t *atomicVar, Cpa32U inValue)
{
#if defined(__linux)
    atomic_set(atomicVar, inValue);
#elif defined(__freebsd)   
    atomic_store_rel_int(atomicVar, inValue);
#endif
}

/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      This function calls the relevant OS code for calculating the
 *      number of clock cycles per operation. 
 *
 *****************************************************************************/

perf_cycles_t
getNumCyclesPerOp(perf_cycles_t numOfCycles, perf_atomic_t *atomicVar, 
                  Cpa32U numOperations)
{
#if defined(__linux)
    do_div(numOfCycles, numOperations);
    return numOfCycles;
#elif defined(__freebsd)
    return ((perf_cycles_t)numOfCycles/atomic_load_acq_int(atomicVar));
#endif
}


/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      Callback function for result of perform operation
 *
 *****************************************************************************/

void performCallback(
        void       *pCallbackTag,
        CpaStatus  status,
        const CpaCySymOp operationType,
        void          *pOpData,
        CpaBufferList *pDstBuffer,
        CpaBoolean    verifyResult)
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
        pPerfData->endCyclesTimestamp = (perf_cycles_t)CLOCK_TIMESTAMP;
        /*
         * once all callbacks are complete
         * finish timestamping global variables
         */
        COMPLETE(&pPerfData->comp);
    }
}


/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      Function for calling random number generation API
 *
 *****************************************************************************/

CpaStatus sampleRandGenPerform(Cpa8U *pWriteRandData, Cpa32U lengthOfRand)
{
    CpaStatus          status        = CPA_STATUS_SUCCESS;
    void               *pCallbackTag = NULL;
    CpaCyRandGenOpData genOpData     = {0};
    CpaFlatBuffer      randData      = {0};

    genOpData.generateBits = CPA_FALSE;
    genOpData.lenInBytes   = lengthOfRand;

    randData.dataLenInBytes = lengthOfRand;
    randData.pData          = pWriteRandData;

    do {
        status = cpaCyRandGen(CPA_INSTANCE_HANDLE_SINGLE,
                        (void *)NULL,
                        pCallbackTag,
                        &genOpData,
                        &randData);
        
        if((status != CPA_STATUS_RETRY) && (status != CPA_STATUS_SUCCESS))
        {
            PRINT_ERR("cpaCyRandGen Random number failed- status = %d\n",
                    status);
            break;
        }

    } while (CPA_STATUS_SUCCESS != status);

    return status;
}


/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      Function for executing relevant algorithm and packet size
 *
 *****************************************************************************/

CpaStatus samplePerformance(CpaStatus (*functionPointer)(Cpa32U))
{

    Cpa8U      loopIteration  = 0;
    CpaStatus  status         = CPA_STATUS_SUCCESS;


    const Cpa32U packetSize[] = {64, 128, 256, 512, 1024, 2048, 4096 };

    for( loopIteration = 0;
         loopIteration < (sizeof(packetSize)/sizeof(uint32_t));
         loopIteration++ )
    {
        status = functionPointer(packetSize[loopIteration] );

        if(CPA_STATUS_SUCCESS != status)
        {
            PRINT_INFO("Tests FAIL\n");
            return  status;
        }
    }

    PRINT_INFO("All Packet Sizes Complete\n");

  return status;
}
