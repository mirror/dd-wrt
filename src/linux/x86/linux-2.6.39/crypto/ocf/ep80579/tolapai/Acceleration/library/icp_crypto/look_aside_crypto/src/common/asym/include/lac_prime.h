/******************************************************************************
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
 *****************************************************************************/

/**
 ***************************************************************************
 * @file lac_prime.h 
 *
 * @defgroup Lac_Prime     Prime
 *
 * @ingroup LacAsym
 *
 * Interfaces exposed by the Prime Component
 * 
 * @lld_start
 *
 * @lld_overview
 * This is the Prime feature implementation. It implements four methods for 
 * primality test: GCD, Fermat test, Miller-Rabin and Lucas test. The client may 
 * require combined testing, i.e. up to one GCD + up to one Fermat + up to
 * 50 Miller-Rabin rounds + up to one Lucas.
 * 
 * To perform GCD test on prime candidate the QAT requires product of small
 * primes. This input is supplied internally by this component i.e. it is 
 * allocated and managed internally so the client is not aware of it.
 *
 * For Miller-Rabin test the client supplies the array of random numbers. 
 * Further processing requires a flat buffer to be associated with each random
 * number. The memory for these flat buffers is assigned internally, as needed,
 * from the pre-allocated memory pool. 
 *
 * In addition to the standard check on the parameters supplied by the client,
 * the prime candidate of the acceptable length is tested for MSB and LSB (non-
 * -even number). For Miller-Rabin method, the number of requested rounds must
 * not exceed 50, the supplied random numbers must match the prime candidate in 
 * length and have to fall in within 1 and prime-1 (1 < random < prime-1)
 * boundaries.
 *
 * The requests for service are created and chained as necessary for multiple 
 * tests in the increasing complexity order: GCD->Fermat->Miller-Rabin->Lucas.
 * The internal functions are called to calculate the function ID for each 
 * method and to construct the input/output argument lists before calling the
 * PKE QAT Comms layer to create the request. After all requests have been
 * created, the PKE QAT Comms layer is called to propagate the requests to the
 * QAT.
 *
 * Buffer alignment is handled by the PKE QAT Comms layer.
 *
 * @lld_dependencies
 * - \ref LacPkeQatComms "PKE QAT Comms" : For creating and sending messages
 * to the QAT
 * - \ref LacMem "Mem" : For memory allocation and freeing, and translating
 * between scalar and pointer types
 * - OSAL : For atomics and logging
 *
 * @lld_initialisation
 * On initialization this component allocates the product-of-small-primes
 * parameter psp1 for GCD test and the memory pool for flat buffers used for 
 * Miller-Rabin test. It also clears the stats.
 *
 * @lld_module_algorithms
 * LacPke_CreateRequest() function takes the parameters for a PKE QAT request,
 * creates the request, aligns the input & output buffer parameters, and fills
 * in the PKE fields.  The request can subsequently be sent to the QAT using
 * LacPke_SendRequest(). In the event of an error this function will tidy up any
 * resources associated with request handle and set it to PKE_INVALID_HANDLE.
 * When the chain of requests is formed (with the same requestHandle) each
 * request structure gets a pointer filled in to point to the next structure
 * (PKE request).
 *
 * LacPke_SendRequest() function sends a PKE request, previously created using
 * LacPke_CreateRequest(), to the QAT. It does NOT block waiting for a response.
 * In the case of synchronous mode the blocking is done elsewhere in the code
 * using wait-queues, and the callback method is handled internally. The
 * callback function is invoked when the response from the QAT has been
 * processed. When a chain of requests is formed, this function sends the
 * request - head of the chain to the QAT. The QAT performs the required
 * operation, in this case the flavour of Prime test. In the event of a
 * successful operation (positive result) the QAT checks if there is a pointer
 * to point to the next request structure, automatically invokes the next
 * request, performs the operation and so on.
 *
 * If any operation returns a negative result then the QAT sends a negative
 * respond back regardless of reaching the end request or not. Similarly,
 * only when all operations result in a positive outcome and the QAT reaches
 * the end of the chain and only then is able to send a positive response.
 *
 * @note
 * The Prime feature may be called in Asynchronous or Synchronous modes.
 * In Asynchronous mode the user supplies a Callback function to the API.
 * Control returns to the client after the message has been sent to the QAT and
 * the Callback gets invoked when the QAT completes the operation. There is NO
 * BLOCKING. This mode is preferred for maximum performance.
 * In Synchronous mode the client supplies no Callback function pointer (NULL)
 * and the point of execution is placed on a wait-queue internally, and this is
 * de-queued once the QAT completes the operation. Hence, Synchronous mode is
 * BLOCKING. So avoid using in an interrupt context. To achieve maximum
 * performance from the API Asynchronous mode is preferred. 
 * 
 * @lld_process_context
 *
 * @lld_end
 *
 ***************************************************************************/


/*****************************************************************************/

#ifndef LAC_PRIME_H
#define LAC_PRIME_H

/* 
******************************************************************************
* Include public/global header files 
******************************************************************************
*/ 

#include "cpa.h"

/* Include Osal files */
#include "IxOsal.h"

/* Include QAT files */
#include "icp_qat_fw_mmp.h"
#include "icp_qat_fw_mmp_ids.h"


/* 
*******************************************************************************
* Include private header files 
*******************************************************************************
*/ 


/*** Types definitions ***/
typedef enum
{
    LAC_PRIME_TEST_START_DELIMITER,
    LAC_PRIME_GCD,
    LAC_PRIME_FERMAT,
    LAC_PRIME_MILLER_RABIN,
    LAC_PRIME_LUCAS,
    LAC_PRIME_TEST_END_DELIMITER
} lac_prime_test_t;
 
/**
 *******************************************************************************
 * @ingroup Lac_Prime 
 *      print the Prime stats to standard output
 *
 * @description
 *      For each engine this function copies the stats using the function
 *      cpaCyPrimeQueryStats. It then prints contents of this structure to
 *      standard output.
 *
 * @see cpaCyPrimeQueryStats()
 *
 *****************************************************************************/
void
LacPrime_StatsShow(void);

Cpa32U
LacPrime_GetFuncID(
    lac_prime_test_t testId,
    Cpa32U dataLenInBits);

void
LacPrime_PopulateParam(
    lac_prime_test_t testId,
    icp_qat_fw_mmp_input_param_t *pIn,
    icp_qat_fw_mmp_output_param_t *pOut,
    CpaCyPrimeTestOpData *pOpData,
    const CpaFlatBuffer *pInputMillerRabinBuffer,
    const CpaFlatBuffer *pGcdProductOfSmallPrimes);

CpaStatus
LacPrime_ParameterCheck(
    CpaCyPrimeTestCbFunc pCb,
    CpaCyPrimeTestOpData *pOpData,
    CpaBoolean *pTestPassed);

void
LacPrime_TestCallback(
    CpaStatus status,
    CpaBoolean pass,
    CpaInstanceHandle instanceHandle,
    lac_pke_op_cb_data_t *pCbData);

#endif /* LAC_PRIME_H */

