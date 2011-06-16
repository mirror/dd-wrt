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
 * @file cpa_perf_defines.h
 *
 * @defgroup Macro definitions
 *
 * @ingroup perfCode
 *
 * @description
 * Contains function prototypes and #defines used throughout code
 * and macros for printing
 *
 ***************************************************************************/
#if defined(__linux) 
#include <linux/types.h>
#include <linux/slab.h>
#include <asm-generic/atomic.h>
#include <linux/sched.h>
#include <linux/module.h>
#elif defined(__freebsd)
#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <machine/cpu.h>
#include <sys/malloc.h>
#include <sys/queue.h>
#include <sys/taskqueue.h>
#include <sys/sema.h>
#endif

#include "cpa.h"
#include "cpa_cy_sym.h" 

/*
******************************************************************************
* General performance code settings
******************************************************************************
*/

#define NUM_OPERATIONS     1000000
#define NUM_RSA_DECRYPT_OPERATIONS   100000
#define NUM_PRE_ALLOCATED_BUFF_LISTS 100

#define OP_PERFORM_TIMEOUT  10000000 

#define NUM_UNCHAINED_BUFFERS 1

#define RETRY_LIMIT     10000 
#define RSA_RETRY_LIMIT 300

/*
********************************************************************************
* Wait for completion Macros
********************************************************************************
*/

#if defined(__linux)
    #define COMPLETION_STRUCT completion

    #define COMPLETION_INIT(c) init_completion(c)

    #define COMPLETION_WAIT(c, timeout)                         \
        wait_for_completion_interruptible_timeout(c, timeout)

    #define COMPLETE(c) complete(c)

    #define COMPLETION_DESTROY(s)

#elif defined(__freebsd)
    #define COMPLETION_STRUCT sema

    #define COMPLETION_INIT(s) sema_init(s, 0, "");

    #define COMPLETION_WAIT(s, timeout) (sema_timedwait(s, timeout) == 0)

    #define COMPLETE(s) sema_post(s)

    #define COMPLETION_DESTROY(s) sema_destroy(s)
#endif

#if defined(__linux)
    #define CLOCK_TIMESTAMP get_cycles()
#elif defined(__freebsd)
    #define CLOCK_TIMESTAMP get_cyclecount()
#endif


#if defined(__linux)
    typedef atomic_t perf_atomic_t;
    typedef cycles_t perf_cycles_t;
#elif defined(__freebsd)
    typedef Cpa32U perf_atomic_t;
    typedef Cpa64U perf_cycles_t;
#endif

/* Performance Data Structure */
typedef struct perf_data_s{
    /* used to correlate requests and responses so that last request */
    /* can be figured out in the callback to get timestamp */ 
    perf_atomic_t responses;
    perf_atomic_t retries;
    perf_cycles_t startCyclesTimestamp; /* start TS before perform */
    perf_cycles_t endCyclesTimestamp;  
    /* end TS for last perform captured in CB */
    struct COMPLETION_STRUCT comp; 
    /* test is complete and end time been captured */
    Cpa32U numOperations;
}perf_data_t;



/****************************************************************************/

#define MAX_KMALLOC_MEM      (1024 * 128)  
#define MAX_BOUNDARY     (4 * 1024 * 1024)

#define MEM_PADDING(s, a) ( ( a - (s % a)) % a  )

#if defined(__linux)
    #define AVOID_SOFTLOCKUP set_current_state(TASK_INTERRUPTIBLE); \
    schedule_timeout(0 * HZ);
#elif defined(__freebsd)
    #define AVOID_SOFTLOCKUP
#endif

typedef enum E_ALLOC_TYPE
{
    MALLOC          = 1,    /*memory allocated using malloc */
    CONTIGMALLOC    = 2     /*memory allocated using contigmalloc */
    
} alloc_type_t;

/* Information associated with a memory allocation */
typedef struct memAllocInfo_s
{
    void*        mAllocMemPtr;   /* memory addr returned by the kernel */
    Cpa32U       mSize;          /* allocated size */
    alloc_type_t mAllocType;
    
} memAllocInfoStruct_t;


/*
******************************************************************************
* Byte Alignment settings
******************************************************************************
*/

#define BYTE_ALIGNMENT_8    8
#define BYTE_ALIGNMENT_64   64

/*
******************************************************************************
* RSA Key Pair sizes and Public Mod N
******************************************************************************
*/

#define RSA_KEYPAIR_SIZE_IN_BYTES_1024 1024
#define RSA_KEYPAIR_SIZE_IN_BYTES_512   512
#define RSA_KEYPAIR_SIZE_IN_BYTES_128   128
#define RSA_KEYPAIR_SIZE_IN_BYTES_64     64
#define RSA_PRIME_P_Q_SIZE_IN_BYTES_64   64

/*
******************************************************************************
* Hash related performance settings.  HMAC-SHA1
******************************************************************************
*/

#define SHA1_DIGEST_LENGTH     20
#define SHA1_KEY_LENGTH        20
#define SHA1_BLOCK_LENGTH      64 
#define AUTH_KEY_LENGTH_64     64
#define HASH_OFFSET_BYTES       0

/*
******************************************************************************
* Cipher related performance settings.  Triple DES-CBC
******************************************************************************
*/

#define TDES_KEY_LENGTH_BYTES      24
#define TDES_CBC_IV_LENGTH_BYTES    8
#define CIPHER_OFFSET_BYTES         0

/*
*******************************************************************************
* Memory related Macros
*******************************************************************************
*/

#if defined(__linux)
#define OS_MEM_FREE_CHECK(ptr) \
do {                           \
    if (NULL != ptr)           \
    {                          \
        kfree(ptr);            \
    }                          \
}while(0)
#elif defined(__freebsd)
#define OS_MEM_FREE_CHECK(ptr)  \
do {                            \
    if (NULL != ptr)            \
    {                           \
        freebsdMemFree(ptr);    \
    }                           \
}while(0)
#endif

#if defined(__linux)
#define OS_MEM_ALIGNED_FREE(ptr) \
do {                           \
    if (NULL != ptr)           \
    {                          \
        freeAlignedMem(ptr);   \
    }                          \
}while(0)
#elif defined(__freebsd)
#define OS_MEM_ALIGNED_FREE(ptr)  \
do {                            \
    if (NULL != ptr)            \
    {                           \
        freebsdMemFree(ptr);    \
    }                           \
}while(0)
#endif


#if defined(__linux)
    #define dprint printk
#elif defined(__freebsd)
    #define dprint printf
#endif


#define ALIGN_DATA(num, align) \
    ( ((num) + (align) -1) & ~((align) - 1) )

#define LAC_BUFFER_LIST_SIZE_GET(numBuffers) \
    (sizeof(CpaBufferList) + \
    (numBuffers * sizeof(CpaFlatBuffer)))


/*
 * Prints the name of the function and the arguments 
 */
#define PRINT_INFO(args...)                      \
{                                                 \
        dprint(args);                           \
}                                        

/*
 * Prints the name of the function and the arguments 
 */
#define PRINT_ERR(args...)           \
do {                                 \
     dprint("Sample Perf: %s(): ", __func__);     \
     dprint(args);                   \
} while (0);


/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      This function allocates physically aligned memory 
 *
 *****************************************************************************/
void
*allocAlignedMem(Cpa32U size, Cpa32U alignment);

/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      This function frees physically aligned memory 
 *
 *****************************************************************************/
void 
freeAlignedMem(void *ptr);

/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      This function allocates memory and check the status of the alloctation
 *
 *****************************************************************************/
void 
*allocOsMemCheck(Cpa32U memLenBytes, CpaStatus *status);

/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 * This function allocates buffer memory and checks status of the alloctation
 *
 *****************************************************************************/
void 
*bufferMemAlloc(Cpa32U size, CpaStatus *status);

/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 * This function frees buffer memory 
 *
 *****************************************************************************/
void 
bufferMemFree(void *addr,Cpa32U size);

/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      This is the main function for performing Cipher performance tests
 *
 *****************************************************************************/
CpaStatus
sampleCipherPerform( Cpa32U packetSizeInBytes);

/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      This is the main function for performing Hash performance tests 
 *
 *****************************************************************************/
CpaStatus
sampleHashPerform( Cpa32U packetSizeInBytes);

/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      This is the main function for performing Algorithm chaining
 *        performance tests.
 *
 *****************************************************************************/
CpaStatus
sampleAlgoChainPerform( Cpa32U packetSizeInBytes);

/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      This function creates an array of buffer lists each with one flat
 *        buffer.  The size of the array is defined by
 *        NUM_PRE_ALLLOCATED_BUFF_LISTS
 *
 *****************************************************************************/
CpaStatus
sampleCreateBuffers( Cpa32U packetSizeInBytes,
                  CpaFlatBuffer *pFlatBuffArray[],
                  CpaBufferList *pBuffListArray[]);

/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      This function frees the memory allocated for the array of buffer
 *        lists and flat buffers. 
 *
 *****************************************************************************/
void
sampleFreeBuffers(Cpa32U packetSizeInBytes,
                    CpaFlatBuffer *srcBuffPtrArray[],
                    CpaBufferList *srcBuffListArray[]);
/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      This function executes the RSA Decrypt performance test
 *
 *****************************************************************************/
CpaStatus
sampleRsaPerform(void);

/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      This function executes the specified function passed in
 *    
 *
 *****************************************************************************/
CpaStatus
samplePerformance(CpaStatus (*functionPointer)(Cpa32U));

/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      This function calls the API to generate a random number
 *
 *****************************************************************************/
CpaStatus
sampleRandGenPerform(Cpa8U *pWriteRandData, Cpa32U lengthOfRand);

/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      This function handles the requested operations callback
 *
 *****************************************************************************/
void
performCallback(
        void             *pCallbackTag,
        CpaStatus        status,
        const CpaCySymOp operationType,
        void             *pOpData,
        CpaBufferList    *pDstBuffer,
        CpaBoolean       verifyResult);

/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      This function handles the requested RSA operations callback
 *
 *****************************************************************************/

void
rsaDecryptCallback(
    void *pCallbackTag,
    CpaStatus status,
    void *pOpdata,
    CpaFlatBuffer *pOut);

/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      This function allocates memory for FreeBsd 
 *
 *****************************************************************************/

void 
*freebsdMemAlloc (Cpa32U size);

/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      This function frees memory for FreeBsd
 *
 *****************************************************************************/

void
freebsdMemFree (void *ptr);

/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *     This function calculates the number of clcok perf_cycles_t per operation
 *
 *****************************************************************************/

perf_cycles_t
getNumCyclesPerOp(perf_cycles_t numOfCycles, perf_atomic_t *atomicVar,
                  Cpa32U numOperations);
/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      This function acts as a wrapper for linux and FreeBsd for 
 *      perf_atomic_t variables
 *
 *****************************************************************************/

Cpa32U
atomicGet(perf_atomic_t *atomicVar);
/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      This function acts as a wrapper for linux and FreeBsd for 
 *      perf_atomic_t variables 
 *
 *****************************************************************************/

void
atomicInc(perf_atomic_t *atomicVar);
/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      This function acts as a wrapper for linux and FreeBsd for 
 *      perf_atomic_t variables 
 *
 *****************************************************************************/

void
atomicSet(perf_atomic_t *atomicVar, Cpa32U inValue);
