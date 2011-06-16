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
 * @file cpa_cy_rand.h
 * 
 * @defgroup cpaCyRand Random Bit/Number Generation API.
 *
 * @ingroup cpaCy
 * 
 * @description
 *      These functions specify the API for the Cryptographic Random Bit and 
 *      Random number generation. 
 * 
 *****************************************************************************/

#ifndef CPA_CY_RAND_H
#define CPA_CY_RAND_H

#include "cpa_cy_common.h"

/**
 *****************************************************************************
 * @file cpa_cy_rand.h
 * @ingroup cpaCyRand
 *      Random Bit/Number Generator Seed Length
 * 
 * @description
 *      Defines the permitted seed length in bytes that may be used with the
 *      cpaCyRandSeed function. 
 * 
 * @see cpaCyRandSeed 
 *
 *****************************************************************************/
#define CPA_CY_RAND_SEED_LEN_IN_BYTES   (48)

/**
 *****************************************************************************
 * @file cpa_cy_rand.h
 * @ingroup cpaCyRand
 *      Random Data Generator Statistics.
 * @description
 *      This structure contains statistics on the random data generation 
 *      operations. Statistics are set to zero when the component is 
 *      initialized, and are collected per instance.
 * 
 ****************************************************************************/
typedef struct _CpaCyRandStats {
    Cpa32U numRandNumRequests;
    /**<  Total number of successful random number generation requests.*/
    Cpa32U numRandNumRequestErrors;
    /**<  Total number of random number generation requests that had an 
     * error and could not be processed.  */
    Cpa32U numRandNumCompleted;
    /**<  Total number of random number operations that completed 
     * successfully. */
    Cpa32U numRandNumCompletedErrors;
    /**<  Total number of random number operations that could not be 
     * completed successfully due to errors. */
    Cpa32U numRandBitRequests;
    /**<  Total number of successful random bit generation requests.*/
    Cpa32U numRandBitRequestErrors;
    /**<  Total number of random bit generation requests that had an 
     * error and could not be processed.  */
    Cpa32U numRandBitCompleted;
    /**<  Total number of random bit operations that completed 
     * successfully. */
    Cpa32U numRandBitCompletedErrors;
    /**<  Total number of random bit operations that could not be 
     * completed successfully due to errors. */
    Cpa32U numNumSeedRequests;
    /**< Total number of seed operations requests. */
    Cpa32U numRandSeedCompleted;
    /**< Total number of seed operations completed. */
    Cpa32U numNumSeedErrors;
    /**< Total number of seed operation errors. */
} CpaCyRandStats;

/**
 *****************************************************************************
 * @file cpa_cy_rand.h
 * @ingroup cpaCyRand
 *      Random Bit/Number Generation Data.
 * @description
 *      This structure lists the different items that are required in the 
 *      cpaCyRandGen function. The client MUST allocate the memory for this
 *      structure. When the structure is passed into the function, ownership of
 *      the memory passes to the function. Ownership of the memory returns to 
 *      the client when this structure is returned with the callback. 
 * 
 * @note
 *      If the client modifies or frees the memory referenced in this structure
 *      after it has been submitted to the cpaCyRandGen function, and before
 *      it has been returned in the callback, undefined behavior will result.
 *
 *****************************************************************************/
typedef struct _CpaCyRandGenOpData {
    CpaBoolean generateBits;
    /**< When set to CPA_TRUE then the cpaCyRandGen function will 
     * generate random bits which will comply with the ANSI X9.82 Part 1
     * specification. When set to CPA_FALSE random numbers will be produced
     * from the random  bits generated by the hardware. This will be spec
     * compliant in terms of the probability of the random nature of the
     * number returned. */
    Cpa32U lenInBytes;
    /**<  Specifies the length in bytes of the data returned. If the data 
     * returned is a random number, then it is implicit that the random number
     * will fall into the following range: Expressed mathematically, the range 
     * is [2^(lenInBytes*8 - 1) to 2^(lenInBytes*8) - 1]. This is equivalent 
     * to "1000...0000" to "1111...1111" which requires (lenInBytes * 8) bits
     * to represent. The maximum number of random bytes that can be requested
     * is 65535 bytes.*/
} CpaCyRandGenOpData;

/**
 *****************************************************************************
 * @file cpa_cy_rand.h
 * @ingroup cpaCyRand
 *      Random Generator Seed Data.
 * @description
 *      This structure lists the different items that required in the 
 *      cpaCyRandSeed function. The client MUST allocate the memory for this
 *      structure. When the structure is passed into the function, ownership of
 *      the memory passes to the function. Ownership of the memory returns to 
 *      the client when this structure is returned with the callback. 
 * 
 * @note
 *      If the client modifies or frees the memory referenced in this structure
 *      after it has been submitted to the cpaCyRandSeed function, and 
 *      before it has been returned in the callback, undefined behavior will 
 *      result.
 * 
 *****************************************************************************/
typedef struct _CpaCyRandSeedOpData {
    CpaBoolean seedUpdate;
    /**< When set to CPA_TRUE then the cpaCyRandSeed function will 
     * update (combine) the specified seed with the stored seed. When set to 
     * CPA_FALSE, the cpaCyRandSeed function will completely discard all 
     * existing entropy in the hardware and replace with the specified seed. */
    CpaFlatBuffer seedData;
    /**< Data for use in either seeding or performing a 
     * seed update. The data that is pointed to are random bits and as such do
     * not have an endian order. For optimal performance the data SHOULD be
     * 8-byte aligned. The length of the seed data is in bytes. This MUST 
     * currently be equal to CPA_CY_RAND_SEED_LEN_IN_BYTES. */
} CpaCyRandSeedOpData;

/**
 *****************************************************************************
 * @file cpa_cy_rand.h
 * @ingroup cpaCyRand
 *      Random Bits or Number Generation Function.
 * 
 * @description
 *      This function is used to request the generation of random bits or a
 *      random number. The generated data and the length of the data will be
 *      returned to the caller in an asynchronous callback function. If random
 *      number generation is selected, the random bits generated by the 
 *      hardware will be converted to a random number that is compliant to the
 *      ANSI X9.82 Part 1 specification. 
 *  
 * @context
 *      When called as an asynchronous function it cannot sleep. It can be 
 *      executed in a context that does not permit sleeping.
 *      When called as a synchronous function it may sleep. It MUST NOT be 
 *      executed in a context that DOES NOT permit sleeping.
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      Yes when configured to operate in synchronous mode.
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param[in]  instanceHandle    Instance handle.
 * @param[in]  pRandGenCb        Pointer to callback function to be invoked
 *                               when the operation is complete. If this is 
 *                               set to a NULL value the function will operate 
 *                               synchronously.
 * @param[in]  pCallbackTag      Opaque User Data for this specific call. Will
 *                               be returned unchanged in the callback.
 * @param[in]  pRandGenOpData    Structure containing all the data needed to 
 *                               perform the random bit/number operation. The 
 *                               client code allocates the memory for this
 *                               structure. This component takes ownership of
 *                               the memory until it is returned in the 
 *                               callback.
 * @param[out] pRandData         Pointer to the memory allocated by the client
 *                               where the random data will be written to. For
 *                               optimal performance, the data pointed to SHOULD
 *                               be 8-byte aligned. There is no endianness 
 *                               associated with the random data.
 *                               On invocation the callback function will
 *                               contain this parameter in it's pOut parameter.
 * 
 * @retval CPA_STATUS_SUCCESS       Function executed successfully.
 * @retval CPA_STATUS_FAIL          Function failed.
 * @retval CPA_STATUS_RETRY          Resubmit the request.
 * @retval CPA_STATUS_INVALID_PARAM Invalid parameter passed in.
 * @retval CPA_STATUS_RESOURCE      Error related to system resources. One 
 *                                  reason may be for an entropy test failing. 
 * 
 * @pre
 *      The component has been initialized via cpaCyStartInstance function.
 * @post
 *      None
 * @note
 *      When pRandGenCb is non-NULL an asynchronous callback of type 
 *      CpaCyRandGenCbFunc is generated in response to this function call. Any
 *      errors generated during processing are reported as part of the callback
 *      status code. Entropy testing and reseeding are performed automatically
 *      by this function.
 * 
 * @see
 *      CpaCyGenFlatBufCbFunc, CpaCyRandGenOpData, cpaCyRandSeed().
 *      
 ******************************************************************************/
CpaStatus 
cpaCyRandGen(const CpaInstanceHandle instanceHandle,
        const CpaCyGenFlatBufCbFunc pRandGenCb,
        void *pCallbackTag,
        const CpaCyRandGenOpData *pRandGenOpData,
        CpaFlatBuffer *pRandData);

/**
 *****************************************************************************
 * @file cpa_cy_rand.h
 * @ingroup cpaCyRand
 *      Random Data Generator Seed Function.
 * 
 * @description
 *      This function is used to either seed or perform a seed update on the 
 *      random data generator. Replacing the seed with a user supplied seed
 *      value, or performing a seed update are completely optional operations. 
 *      If seeding is specified, it has the effect or disregarding all existing
 *      entropy within the random data generator and replacing with the 
 *      specified seed. If performing a seed update, then the specified seed is
 *      mixed into the stored seed. The seed length MUST be equal to
 *      CPA_CY_RAND_SEED_LEN_IN_BYTES. 
 *  
 * @context
 *      When called as an asynchronous function it cannot sleep. It can be 
 *      executed in a context that does not permit sleeping.
 *      When called as a synchronous function it may sleep. It MUST NOT be 
 *      executed in a context that DOES NOT permit sleeping.
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      Yes when configured to operate in synchronous mode.
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param[in] instanceHandle       Instance handle.
 * @param[in] pRandSeedCb          Pointer to callback function to be invoked
 *                                 when the operation is complete. If this is 
 *                                 set to a NULL value the function will operate 
 *                                 synchronously.
 * @param[in] pCallbackTag         Opaque User Data for this specific call. Will
 *                                 be returned unchanged in the callback.
 * @param[in] pSeedOpData          Structure containing all the data needed to 
 *                                 perform the random generator seed operation. 
 *                                 The client code allocates the memory for this
 *                                 structure. This component takes ownership of 
 *                                 the memory until it is returned in the
 *                                 callback.
 *              
 * @retval CPA_STATUS_SUCCESS        Function executed successfully.
 * @retval CPA_STATUS_FAIL           Function failed.
 * @retval CPA_STATUS_RETRY          Resubmit the request.
 * @retval CPA_STATUS_INVALID_PARAM  Invalid parameter passed in.
 * @retval CPA_STATUS_RESOURCE       Error related to system resources.
 * 
 * @pre
 *      The component has been initialized via cpaCyStartInstance function.
 * @post
 *      None
 * @note
 *      When pRandSeedCn is non-NULL an asynchronous callback of type 
 *      CpaCyRandSeedCbFunc is generated in response to this function call. Any
 *      errors generated during processing are reported as part of the callback
 *      status code. Entropy testing and reseeding are performed automatically
 *      by the cpaCyRandGen function. 
 * 
 * @see
 *      CpaCyGenericCbFunc, CpaCyRandSeedOpData, cpaCyRandGen()
 *      
 ******************************************************************************/
CpaStatus 
cpaCyRandSeed(const CpaInstanceHandle instanceHandle,
        const CpaCyGenericCbFunc pRandSeedCb,
        void *pCallbackTag,
        const CpaCyRandSeedOpData *pSeedOpData);

/**
 *****************************************************************************
 * @file cpa_cy_rand.h
 * @ingroup cpaCyRand
 *      Query random number statistics specific to an instance.
 * 
 * @description
 *      This function will query a specific instance for random number 
 *      statistics. The user MUST allocate the CpaCyRandStats structure
 *      and pass the reference to that into this function call. This function 
 *      will write the statistic results into the passed in 
 *      CpaCyRandStats structure. 
 * 
 *      Note: statistics returned by this function do not interrupt current data
 *      processing and as such can be slightly out of sync with operations that
 *      are in progress during the statistics retrieval process
 *
 * @context
 *      This is a synchronous function and it can sleep. It MUST NOT be 
 *      executed in a context that DOES NOT permit sleeping.
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      This function is synchronous and blocking.
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param[in]  instanceHandle        Instance handle.
 * @param[out] pRandStats            Pointer to memory into which the statistics 
 *                                   will be written. 
 *
 * @retval CPA_STATUS_SUCCESS        Function executed successfully.
 * @retval CPA_STATUS_FAIL           Function failed.
 * @retval CPA_STATUS_INVALID_PARAM  Invalid parameter passed in.
 * @retval CPA_STATUS_RESOURCE       Error related to system resources.
 * 
 * @pre
 *      Component has been initialized.
 * @post
 *      None
 * @note
 *      This function operates in a synchronous manner and no asynchronous 
 *      callback will be generated.
 * @see
 *      CpaCyRandStats
 * 
 * 
 *****************************************************************************/
CpaStatus 
cpaCyRandQueryStats(const CpaInstanceHandle instanceHandle,
        CpaCyRandStats *pRandStats);

/*****************************************************************************/
#endif /* CPA_CY_RAND_H */
/*****************************************************************************/
