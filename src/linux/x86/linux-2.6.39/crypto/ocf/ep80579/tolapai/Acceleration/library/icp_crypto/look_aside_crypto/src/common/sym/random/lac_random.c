/*************************************************************************
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
 *****************************************************************************
 * @file lac_random.c
 *
 * @ingroup LacSym_Random
 *
 * Implementation of the Random Interfaces 
 * 
 *****************************************************************************/

/* 
*******************************************************************************
* Include public/global header files 
*******************************************************************************
*/
#include "cpa.h"

#include "cpa_cy_rand.h"
#include "cpa_cy_common.h"
#include "lac_sym.h"

#include "icp_qat_fw_la.h"
#include "qat_comms.h"
#include "qatal_rand.h"

/* 
*******************************************************************************
* Include private header files 
*******************************************************************************
*/
#include "lac_common.h"
#include "lac_mem.h"
#include "lac_sym_qat.h"
#include "lac_random.h"
#include "lac_log.h"
#include "lac_hooks.h"

#include "lac_module.h"

/*
*******************************************************************************
* Macros & Constants
*******************************************************************************
*/      

#define LAC_RAND_NUM_STATS (sizeof(CpaCyRandStats) / sizeof(Cpa32U))
/**< Number of random statistics */

#define LAC_RAND_STAT_INC(statistic)                                    \
(ixOsalAtomicInc(&lacRandStatsArr[ offsetof(CpaCyRandStats, statistic)  \
                                   / sizeof(Cpa32U) ]))
/**< Statistics incrementation macro */

#define LAC_RAND_NUM_SET_MSB_IN_BYTE (0x80) 
/**< Bit field to be OR'd with the MSb of generated random number */

#define LAC_RAND_NUM_MAX_LENGTH_IN_BYTES (0xFFFF) 
/**< Maximum lenght in bytes used to generate random numbers */    

#define LAC_RAND_NUM_MAX_FULL_CACHE_SIZE_IN_BYTES (131070)
/**< Maximum cache size in bytes used to generate random numbers in 
 * synchronous mode */ 

#define LAC_RAND_NUM_MAX_CACHE_SIZE_IN_BYTES \
    ((LAC_RAND_NUM_MAX_LENGTH_IN_BYTES) * (2))
/**< Maximum size of the full cache of random numbers in bytes. This size is
 * equal to twice the maximum lenght used to generate random numbers */ 

#define LAC_RAND_NUM_ENTROPY_THRESHOLD (10)
/**< Enthropy threshold */
#define LAC_RAND_NUM_MAX_ENTROPY_TESTS (1000)
/**< Maximum value of failing entropies returned before considering that an
 * error occured in random */

#define LAC_RAND_NUM_CACHES (2)
/**< Number of random caches */
#define LAC_RAND_NUM_CACHE_ONE (1)
/**< Number of random cache one */

#define LAC_RAND_NUM_CACHE_OK (0)
#define LAC_RAND_NUM_CACHE_REGENERATING (1)
#define LAC_RAND_NUM_CACHE_REGENERATING_RETRY (2)
/**< States used for the regeneration of the cache of random numbers */

/*
 * Values below used to wait on completion of random generation liveness request
 * 10-second timeout chosen, to avoid infinite wait
 */
#define LAC_RAND_NUM_WAIT_COUNT (1000)
/**< Number of times to sleep and wakeup */
#define LAC_RAND_NUM_TIMEOUT_IN_MS (10)
/**< Time to sleep in ms */

/*
*******************************************************************************
* Type Definitions
*******************************************************************************
*/      

/**< Rand session objects used to keep track of invocations and 
 * callbacks for random generation */
typedef struct icp_lac_rand_gen_session_s
{
    CpaCyGenFlatBufCbFunc pRandCb;
    CpaInstanceHandle instanceHandle;
    void *pCallbackTag;
    CpaCyRandGenOpData *pOpData;
} icp_lac_rand_gen_session_t;


/**< Rand objects used to generate random numbers in synchronous mode */
typedef struct random_synchronous_s
{
    /* Cache of preallocated random numbers */
    CpaFlatBuffer cacheRandData;
    
    /* Used to regenerate the cache of random numbers */
    CpaCyRandGenOpData randGenOpData;
    
    /* Used to know if one side of the cache has to be regenerated */
    Cpa8U cacheState;
} random_synchronous_t;

/*
*******************************************************************************
* Global / Static Variables
*******************************************************************************
*/      

static random_synchronous_t lacRandCache[LAC_RAND_NUM_CACHES];
/**< Two different caches are used to allow continuing giving 
 * random numbers even if regenerating one of the cache */

static volatile Cpa8U lacRandCurrentCache = 0;
/**< Current cache in use in synchronous mode */

STATIC IxOsalAtomic lacRandStatsArr[LAC_RAND_NUM_STATS];
/**< Array of statistics */

static lac_lock_t lacRandLock;
/**< Lock used to protect the global variables in multi threaded mode */

static volatile Cpa32U lacRandCacheReadIndex = 0;
/**< This index is used to keep track of the next available random numbers to 
  * give in synchronous mode */

static Cpa32U lacRandCacheSize = LAC_RAND_NUM_MAX_FULL_CACHE_SIZE_IN_BYTES;
/**< The cacheSize has to be defined by the user as at least twice the maximum 
 * lenght in bytes value they will expect to generate in synchronous mode. 
 * A synchronous mode generation of more than half of this value can't be 
 * done by LAC.
 * The cacheSize is provided by the ASD component at LAC initialisation */

static volatile CpaBoolean lacRandRestartGatherState = CPA_TRUE;
/**< Used for statistical entropy testing */

static CpaBoolean lacRandIsEntropyPassing = CPA_TRUE;
/**< Entropy passing flag */

static Cpa32U lacRandEntropyNumOperationCounter;
static Cpa32U lacRandEntropyFailingCounter;
/**< Counters used to keep track of the states of the entropy statistical tests.
 * We don't need to use atomic counters as these counters are only use in an 
 * interrupt context inside the callbacks and can't be overlaped */


/*
*******************************************************************************
* Define public/global function definitions 
*******************************************************************************
*/

/* Function declaration needed as called in "LacRand_RegenerateBuffer" */
static CpaStatus
LacRand_GenInternal (CpaInstanceHandle instanceHandle,
                     CpaCyGenFlatBufCbFunc pRandGenCb,
                     void *pCallbackTag,
                     const CpaCyRandGenOpData *pRandGenOpData,
                     CpaFlatBuffer *pRandData);

/**
 * A single bit stream of 20,000 consecutive bits of output 
 * from the generator should be subjected to this test
 */
CpaStatus
LacRand_DisableDrbg(lac_sym_random_cookie_t *pCookie)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    
    icp_qat_fw_la_rng_req_t msg;
                                   
    lacRandIsEntropyPassing = CPA_FALSE;
        
    LAC_OS_BZERO(&msg, sizeof(msg));
    LAC_MEM_SHARED_WRITE_8BIT(msg.comn_la_req.la_cmd_id,
                     ICP_QAT_FW_LA_CMD_DRBG_DISABLE);

    /* session struct address is passed in message so that it can be    
     * accessed later by the response message callback handler */
    LAC_MEM_SHARED_WRITE_FROM_PTR(msg.comn_req.opaque_data, pCookie);
        
    status = QatComms_ReqHdrCreate(&msg, ICP_ARCH_IF_REQ_QAT_FW_LA);

    if (CPA_STATUS_SUCCESS == status)
    {
        /* Send message to QAT to disable the DRBG */
        status = QatComms_MsgSend(&msg,
                                  ICP_ARCH_IF_REQ_QAT_FW_LA,
                                  QAT_COMMS_PRIORITY_NORMAL,
                                  pCookie->instanceHandle);
    }

    return status;
}

/**
 * Called by QAT Comms module to process Rand response messages
 * used with the synchronous API
 */        
static void                                   
LacRand_ProcessCallbackSynchronous(void *pCallbackTag,
                                   CpaStatus status,
                                   void *pRandGenOpData,
                                   CpaFlatBuffer *pRandData)
{  
    Cpa8U *pCacheState = (Cpa8U *)pCallbackTag;
    /* Half of the cache buffer had been fully regenerated
     * so reinitialise the state of the cache for next requests */  
     
    if(LAC_RAND_NUM_CACHE_REGENERATING == *pCacheState)
    {
        *pCacheState = LAC_RAND_NUM_CACHE_OK; 
    }
}

/**
 * Called by QAT Comms module to process Rand response messages
 */        
void
LacRand_ProcessCallback(icp_qat_fw_la_cmd_id_t commandId,
                        void *pOpaqueData,
                        icp_qat_fw_comn_flags cmnRespFlags)
{
    CpaStatus responseStatus = CPA_STATUS_SUCCESS;
    CpaStatus statusEntropy = CPA_STATUS_SUCCESS;
    
    if(CPA_FALSE == lacRandIsEntropyPassing)
    {
        LAC_LOG_ERROR("Entropy testing failed");    
        responseStatus = CPA_STATUS_FAIL;
    }

    if (ICP_QAT_FW_LA_DRBG_DISABLED == 
            ICP_QAT_FW_LA_DRBG_STATE_GET(cmnRespFlags))
    { 
        LAC_LOG_ERROR("DRBG disabled");         
        responseStatus = CPA_STATUS_FAIL;
    }
 
    if (ICP_QAT_FW_LA_NRBG_STATUS_FAIL == 
            ICP_QAT_FW_LA_NRBG_STATUS_GET(cmnRespFlags))
    {
        LAC_LOG_ERROR("NRBG failed to get entropy");               
        responseStatus = CPA_STATUS_FAIL;
    }

    /* opaque data contains the address of the session struct memory
     * allocated for this request (which must be freed in this function)
     */
    LAC_ASSERT(pOpaqueData, 
               "LacRand_ProcessCallback() - pOpaqueData is NULL\n");
 
    if (ICP_QAT_FW_LA_CMD_DRBG_GET_RANDOM == commandId)
    {
        lac_sym_random_cookie_t *pCookie = 
            (lac_sym_random_cookie_t *)pOpaqueData;
        CpaCyRandGenOpData * pOpData = (CpaCyRandGenOpData *)
                                           LAC_CONST_PTR_CAST(pCookie->pOpData);
        void *pCallbackTag = pCookie->pCallbackTag;
        CpaCyGenFlatBufCbFunc pRandCb = pCookie->pRandCb;
        CpaFlatBuffer *pRandData = pCookie->pRandData;
        
        /* Check if DRBG is in COMPLETED state and entropy sample collected */
        if(CPA_STATUS_SUCCESS == responseStatus)
        {
            if(ICP_QAT_FW_LA_DRBG_GATHER_STATE_COMPLETED == 
                    ICP_QAT_FW_LA_DRBG_GATHER_STATE_GET(cmnRespFlags))
            {     
                /* Statistical entropy testing */
                /* LAC calls the QAT_AL entropy test API */ 
                /* This function may fails in a normal behaviour so we have to 
                 * keep track of the number of failures to ensure it does not 
                 * go above a threshold */
                if(CPA_STATUS_SUCCESS != icp_QatalRandEntrophyTestRun())
                {
                    /* Increment number of failing entropy */
                    lacRandEntropyFailingCounter++;
                }
             
                /* Increment total number of entropy tests done */
                lacRandEntropyNumOperationCounter++;
                
                /* Test if too many statistical entropy errors */
                if(lacRandEntropyFailingCounter
                    > LAC_RAND_NUM_ENTROPY_THRESHOLD)
                {
                    responseStatus = CPA_STATUS_FAIL;
                    
                    /* Send message to QAT to disable the DRBG */
                    statusEntropy = LacRand_DisableDrbg(pCookie);
                    if(CPA_STATUS_SUCCESS != statusEntropy)
                    {
                        LAC_LOG_ERROR("DRBG not disabled");
                    }
                    
                    LAC_LOG_ERROR("Entropy testing failed");
                }
                else
                {
                    /* Start a new entropy testing */
                    lacRandRestartGatherState = CPA_TRUE;
                }
                
                /* Reset the counters for new testing */
                if(LAC_RAND_NUM_MAX_ENTROPY_TESTS 
                    == lacRandEntropyNumOperationCounter)
                {
                    lacRandEntropyFailingCounter = 0;
                    lacRandEntropyNumOperationCounter = 0;
                }
            }
        }
   

        if (CPA_TRUE == pOpData->generateBits)
        {
            if(pRandCb != LacRand_ProcessCallbackSynchronous)
            {
                if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.random.istat)
                {
                    LAC_RAND_STAT_INC(numRandBitCompleted);

                    if (CPA_STATUS_SUCCESS != responseStatus)
                    {
                        LAC_RAND_STAT_INC(numRandBitCompletedErrors);
                    }
                }
            }
        }
        else
        {
            /* if asked to generate a random number (as opposed to random bits)
             *   force top bit to 1
             */
            pCookie->pRandData->pData[0] |= LAC_RAND_NUM_SET_MSB_IN_BYTE;

            if(pRandCb != LacRand_ProcessCallbackSynchronous)
            {
                if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.random.istat)
                {
                    LAC_RAND_STAT_INC(numRandNumCompleted);
                
                    if(CPA_STATUS_SUCCESS != responseStatus)
                    {
                        LAC_RAND_STAT_INC(numRandNumCompletedErrors);
                    }
                } /* end if collectStatistics */
            }
        }     

        Lac_MemPoolEntryFree(pCookie);
        pRandData->dataLenInBytes = pOpData->lenInBytes;
        pRandCb(pCallbackTag, responseStatus, pOpData, pRandData);
    }
    else if (ICP_QAT_FW_LA_CMD_DRBG_DISABLE != commandId)
    {
        /* Unknown message */
        LAC_LOG_ERROR1("Unknown msg id 0x%x", commandId);
    }
}

/**
 * Internal function used to regenerate the preallocated buffer in 
 * synchronous mode
 */             
static CpaStatus
LacRand_RegenerateBuffer(CpaInstanceHandle instanceHandle, Cpa8U cacheNum)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    void *pCallbackTag;
    CpaCyRandGenOpData *pRandGenOpData;
    CpaFlatBuffer *pRandData;
          
    pRandGenOpData = &(lacRandCache[cacheNum].randGenOpData);
    pRandData = &(lacRandCache[cacheNum].cacheRandData);
    pRandGenOpData->lenInBytes = lacRandCacheSize / LAC_RAND_NUM_CACHES;
    
    pCallbackTag = (void *) &(lacRandCache[cacheNum].cacheState);
        
    /* Populate CpaCyRandGenOpData */    
    pRandGenOpData->generateBits = CPA_TRUE;
    
    /* Generate new random numbers to use in synchronous mode */
    status = LacRand_GenInternal(instanceHandle, 
                                 LacRand_ProcessCallbackSynchronous, 
                                 pCallbackTag, pRandGenOpData,
                                 pRandData);

    return status;
}
   
  
/**
 * Called internally from "LacRand_GenInternal" in synchronous mode
 */ 
static CpaStatus
LacRand_GenSynchronous(CpaInstanceHandle instanceHandle,
                      void *pCallbackTag,
                      const CpaCyRandGenOpData *pRandGenOpData,
                      CpaFlatBuffer *pRandData)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    
    volatile Cpa8U currentCacheCopy = lacRandCurrentCache;
    volatile Cpa8U otherCacheCopy = 1 - currentCacheCopy; 
    volatile Cpa32U cacheReadIndexCopy = 0;
    
    Cpa32U sizeMinusRead = 0;
     
    /* Return an error if the length asked by the user is greater 
     * than the size of the cache buffer */
    if(pRandGenOpData->lenInBytes > lacRandCacheSize)  
    {
        LAC_INVALID_PARAM_LOG("pRandGenOpData->lenInBytes");    
        return CPA_STATUS_INVALID_PARAM;
    }
    
    /* A critical section is used here as we are using global variables
     * which can be used by multiple threads and users */
     
    /* >>>>> BEGIN CRITICAL SECTION <<<<< */
    LAC_SPINLOCK(&lacRandLock);

    /* Get read index and copy it */
    cacheReadIndexCopy = lacRandCacheReadIndex;

    /* Save the currentCache */
    currentCacheCopy = lacRandCurrentCache;
    otherCacheCopy = LAC_RAND_NUM_CACHE_ONE - currentCacheCopy; 

    /* Enough available data in only one buffer. 
     * If the lenght is equal to "lacRandCacheSize/2" we use the other buffer */
    if((cacheReadIndexCopy + pRandGenOpData->lenInBytes) < 
        (lacRandCacheSize / LAC_RAND_NUM_CACHES))
    {      
        /* Increment the read index for next generation and copy data */
        lacRandCacheReadIndex += pRandGenOpData->lenInBytes;
        
        /* >>>>> END CRITICAL SECTION <<<<< */
        LAC_SPINUNLOCK(&lacRandLock);

        memcpy(pRandData->pData, 
         lacRandCache[currentCacheCopy].cacheRandData.pData 
             + cacheReadIndexCopy, 
         pRandGenOpData->lenInBytes);   
    }     
    
    /* Data on the two buffers */
    else
    {
        otherCacheCopy = LAC_RAND_NUM_CACHE_ONE - currentCacheCopy;
        
        /* Check if regenerating on the other buffer */  
        if(LAC_RAND_NUM_CACHE_REGENERATING_RETRY
            == lacRandCache[otherCacheCopy].cacheState)
        {
            /* Regenerate the buffer */
            lacRandCache[otherCacheCopy].cacheState 
                = LAC_RAND_NUM_CACHE_REGENERATING;
            
            /* >>>>> END CRITICAL SECTION <<<<< */
            LAC_SPINUNLOCK(&lacRandLock);
            
            status = LacRand_RegenerateBuffer(instanceHandle, otherCacheCopy);
        
            if(CPA_STATUS_RETRY == status)
            {
                lacRandCache[otherCacheCopy].cacheState 
                    = LAC_RAND_NUM_CACHE_REGENERATING_RETRY;
            }
            
            if((CPA_STATUS_SUCCESS == status)||(CPA_STATUS_RETRY == status))
            {
                /* Return retry as we didn't actually give data to the user */
                return CPA_STATUS_RETRY;
            }
            else
            {
                return status;
            }   
        }
        else if(LAC_RAND_NUM_CACHE_REGENERATING 
            == lacRandCache[otherCacheCopy].cacheState)
        {
            /* >>>>> END CRITICAL SECTION <<<<< */
            LAC_SPINUNLOCK(&lacRandLock);

            return CPA_STATUS_RETRY;
        }      
        else
        {           
            /* Increment the read index for next generation and copy data */ 
            sizeMinusRead = ((lacRandCacheSize / LAC_RAND_NUM_CACHES) 
                - cacheReadIndexCopy);
            lacRandCacheReadIndex = pRandGenOpData->lenInBytes - sizeMinusRead;
            
            /* Save the current cache */
            currentCacheCopy = lacRandCurrentCache;

            /* Swap current cache to take available random numbers in the 
             * other cache */
            lacRandCurrentCache = otherCacheCopy;
           
            lacRandCache[currentCacheCopy].cacheState
                = LAC_RAND_NUM_CACHE_REGENERATING;
            
            /* >>>>> END CRITICAL SECTION <<<<< */
            LAC_SPINUNLOCK(&lacRandLock);
           
            if(sizeMinusRead > 0)
            {
                memcpy(pRandData->pData, 
                    lacRandCache[currentCacheCopy].cacheRandData.pData 
                    + cacheReadIndexCopy, sizeMinusRead);
            }
            if((pRandGenOpData->lenInBytes - sizeMinusRead) > 0)
            {
                memcpy(pRandData->pData + sizeMinusRead, 
                    lacRandCache[otherCacheCopy].cacheRandData.pData, 
                    pRandGenOpData->lenInBytes - sizeMinusRead);
            } 
                       
            status = LacRand_RegenerateBuffer(instanceHandle, currentCacheCopy);
            
            if(CPA_STATUS_RETRY == status)
            {
                lacRandCache[currentCacheCopy].cacheState  
                    = LAC_RAND_NUM_CACHE_REGENERATING_RETRY;
                    
                /* The user request is successfull, the retry will be handled 
                 * on the next user request */    
                status = CPA_STATUS_SUCCESS;                 
            }                          
        }   
    }

    if(CPA_FALSE == pRandGenOpData->generateBits)
    {     
        /* if asked to generate a random number (as opposed to random bits)
         * force top bit to 1 
         */
        pRandData->pData[0] |= LAC_RAND_NUM_SET_MSB_IN_BYTE;
    }

    return status;      
}

/**
 * Called internally from "cpaCyRandGen" to request a new random number to 
 * be generated
 * or to use a random number pre-generated in the cache in synchronous mode
 */  
static CpaStatus
LacRand_GenInternal (CpaInstanceHandle instanceHandle,
                     CpaCyGenFlatBufCbFunc pRandGenCb,
                     void *pCallbackTag,
                     const CpaCyRandGenOpData *pRandGenOpData,
                     CpaFlatBuffer *pRandData)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    lac_sym_random_cookie_t *pCookie = NULL;

    icp_qat_fw_la_rng_req_t msg;
     
    /* Synchronous mode */
    if(NULL == pRandGenCb)
    {                      
        status =  LacRand_GenSynchronous(instanceHandle,
                                         pCallbackTag,
                                         pRandGenOpData,
                                         pRandData);
                                      
        if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.random.istat)
        {
            if(CPA_STATUS_SUCCESS == status)
            {
                if(CPA_TRUE == pRandGenOpData->generateBits)
                {       
                    LAC_RAND_STAT_INC(numRandBitCompleted);
                }
                else
                {
                    LAC_RAND_STAT_INC(numRandNumCompleted);
                }       
            } 
        }
      
        return status;                               
    }
    else
    {
        /* Allocate & populate session descriptor
         * The memory is freed in callback unless error occurs here
         */
        if((pCookie =
         (lac_sym_random_cookie_t*)Lac_MemPoolEntryAlloc(lac_sym_cookie_memory))
         == NULL)
        {
            LAC_LOG_ERROR("Cannot get mem pool entry");
            return CPA_STATUS_RESOURCE;
        }

        pCookie->pRandCb = pRandGenCb;
        pCookie->instanceHandle = instanceHandle;
        pCookie->pCallbackTag = pCallbackTag;
        pCookie->pOpData = pRandGenOpData;
        pCookie->pRandData = pRandData;
    
        /* Populate the request message (clear the contents first) */
        LAC_OS_BZERO(&msg, sizeof(msg));
        LAC_MEM_SHARED_WRITE_8BIT(msg.comn_la_req.la_cmd_id,
                             ICP_QAT_FW_LA_CMD_DRBG_GET_RANDOM);

        LAC_MEM_SHARED_WRITE_VIRT_TO_PHYS_PTR(msg.comn_req.dest_data_addr,
                                              pRandData->pData);
        LAC_MEM_SHARED_WRITE_16BIT(msg.length, pRandGenOpData->lenInBytes); 
        
        /* >>>>> BEGIN CRITICAL SECTION <<<<< */
        LAC_SPINLOCK(&lacRandLock);
    
        if(CPA_TRUE == lacRandRestartGatherState)
        {
            LAC_MEM_SHARED_WRITE_8BIT(msg.gather_state, 
                ICP_QAT_FW_LA_DRBG_GATHER_STATE_RESTART);
            lacRandRestartGatherState = CPA_FALSE;
        }
        else
        {
            LAC_MEM_SHARED_WRITE_8BIT(msg.gather_state, 
                ICP_QAT_FW_LA_DRBG_GATHER_STATE_IGNORE);   
        }
        
        /* >>>>> END CRITICAL SECTION <<<<< */
        LAC_SPINUNLOCK(&lacRandLock);   
        
        /* session struct address is passed in message so that it can be
         * accessed later by the response message callback handler
         * see LacRand_ProcessCallback() for details
         */
        LAC_MEM_SHARED_WRITE_FROM_PTR(msg.comn_req.opaque_data, pCookie);

        status = QatComms_ReqHdrCreate(&msg, ICP_ARCH_IF_REQ_QAT_FW_LA);

        if (CPA_STATUS_SUCCESS == status)
        {
            /* Send message to QAT */
            status = QatComms_MsgSend(&msg,
                                      ICP_ARCH_IF_REQ_QAT_FW_LA,
                                      QAT_COMMS_PRIORITY_NORMAL,
                                      instanceHandle);
        }
        
        if (CPA_STATUS_SUCCESS != status)
        {
            if (NULL != pCookie)
            {
                Lac_MemPoolEntryFree(pCookie);
            } 
        }
     
    }
    
    return status;                  
}       
      

/**
 *****************************************************************************
 * @ingroup LacSym_Random
 *
 * Called from user API to request a new random number to be generated
 * or to use a random number pre-generated in the cache in synchronous mode
 *****************************************************************************/
CpaStatus
cpaCyRandGen (const CpaInstanceHandle instanceHandle,
              const CpaCyGenFlatBufCbFunc pRandGenCb,
              void *pCallbackTag,
              const CpaCyRandGenOpData *pRandGenOpData,
              CpaFlatBuffer *pRandData)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    CpaBoolean generateBits = CPA_FALSE;
    
    /* Check if LAC is initialised otherwise return an error */
    LAC_INITIALISED_CHECK();
 
    /* Block the generation if the statistical entropy test fails */ 
    if(CPA_FALSE == lacRandIsEntropyPassing)
    {           
        LAC_LOG_ERROR("Statistical entropy is failing");
        return CPA_STATUS_FAIL;
    }  
        
    LAC_CHECK_NULL_PARAM(pRandGenOpData);
    LAC_CHECK_NULL_PARAM(pRandData);
    LAC_CHECK_NULL_PARAM(pRandData->pData);

    if ((0 == pRandGenOpData->lenInBytes) || 
        (pRandGenOpData->lenInBytes > LAC_RAND_NUM_MAX_LENGTH_IN_BYTES))
    {
        LAC_INVALID_PARAM_LOG("Invalid Param : pRandGenOpData->lenInBytes");          
        return CPA_STATUS_INVALID_PARAM;
    }
 
    generateBits = pRandGenOpData->generateBits;

    status = LacRand_GenInternal (instanceHandle,
                                  pRandGenCb,
                                  pCallbackTag,
                                  pRandGenOpData,
                                  pRandData);
              
    if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.random.istat)
    {
        if (CPA_STATUS_SUCCESS == status)
        {
            if (CPA_TRUE == generateBits)
            {
                LAC_RAND_STAT_INC(numRandBitRequests);
            }
            else
            {
                LAC_RAND_STAT_INC(numRandNumRequests);
            }
        }
        else
        {
            if (CPA_TRUE == generateBits)
            {
                LAC_RAND_STAT_INC(numRandBitRequests);
                LAC_RAND_STAT_INC(numRandBitRequestErrors);
            }
            else
            {
                LAC_RAND_STAT_INC(numRandNumRequests);
                LAC_RAND_STAT_INC(numRandNumRequestErrors);
            }
        }
    } /* endif collectStatistics */
    return status;                            
}

/**
 * Initialization function for the cache size of the Random Number Generator 
 * component
 *
 * @warning this function is not reentrant nor thread safe 
 * and has to be called before LacRand_Init
 */
CpaStatus 
LacRand_InitCacheSize(Cpa32U lacRandomCacheSize)
{
    lacRandCacheSize = lacRandomCacheSize;
    
    if(0 == lacRandCacheSize)
    {
        LAC_INVALID_PARAM_LOG("Invalid Param : lacRandCacheSize");
        return CPA_STATUS_INVALID_PARAM;
    } 
    
    /* The maximum allowed cache size is LAC_RAND_NUM_MAX_CACHE_SIZE_IN_BYTES
     * (2 full lac random generations) */
    if(lacRandCacheSize > LAC_RAND_NUM_MAX_CACHE_SIZE_IN_BYTES)
    {
        LAC_INVALID_PARAM_LOG("Invalid Param : lacRandCacheSize");
        return CPA_STATUS_INVALID_PARAM;
    }

    return CPA_STATUS_SUCCESS;
}


/**
 * Initialization function for the Random Number Generator component
 *
 * @warning this function is not reentrant nor thread safe
 */
CpaStatus 
LacRand_Init(void)
{     
    CpaStatus status = CPA_STATUS_SUCCESS;
    
    void *pCallbackTag = 0;
    Cpa8U cacheNum = 0;
    Cpa32U count = 0;

    LAC_SPINLOCK_INIT(&lacRandLock);

    /* Reset statistics counters */
    LAC_OS_BZERO(&lacRandStatsArr, sizeof(lacRandStatsArr));
    lacRandEntropyNumOperationCounter = 0;
    lacRandEntropyFailingCounter = 0;

    /* Initialise the index on the cache buffer to use in synchronous mode */
    lacRandCacheReadIndex = 0;
  
    /* Initialise the statistical entropy testing parameters */
    lacRandIsEntropyPassing = CPA_TRUE;
    lacRandRestartGatherState = CPA_TRUE;

    LacSymQat_RespHandlerRegister(ICP_QAT_FW_LA_CMD_DRBG_GET_RANDOM,
                                  LacRand_ProcessCallback);
        
    LacSymQat_RespHandlerRegister(ICP_QAT_FW_LA_CMD_DRBG_DISABLE,
                                  LacRand_ProcessCallback);

    for(cacheNum = 0; cacheNum < LAC_RAND_NUM_CACHES; cacheNum++)
    {
        /* Generate random numbers */    
        lacRandCache[cacheNum].randGenOpData.generateBits = CPA_TRUE;
        lacRandCache[cacheNum].randGenOpData.lenInBytes
            = lacRandCacheSize / LAC_RAND_NUM_CACHES;
            
        /* Allocate memory for the rand_operation_data_t structure */  
        status = LAC_OS_CAMALLOC(
            &(lacRandCache[cacheNum].cacheRandData.pData), 
            (lacRandCacheSize / LAC_RAND_NUM_CACHES), sizeof(int));
            
        if (CPA_STATUS_SUCCESS != status)
        {
            LAC_LOG_ERROR("Failed to allocate random data buffer");

            status = CPA_STATUS_FAIL;
            break;
        }
        
        /* Initialise the cache of random numbers to use in synchronous 
         * mode */
        lacRandCache[cacheNum].cacheState = LAC_RAND_NUM_CACHE_REGENERATING;
                  
        pCallbackTag = (void *) 
            &(lacRandCache[cacheNum].cacheState);
        
        /* Preallocate random numbers to use in synchronous mode */
        status = LacRand_GenInternal(CPA_INSTANCE_HANDLE_SINGLE, 
                                     LacRand_ProcessCallbackSynchronous, 
                                     pCallbackTag,
                                     &(lacRandCache[cacheNum].randGenOpData),
                                     &(lacRandCache[cacheNum].cacheRandData));
                                            
        if (CPA_STATUS_SUCCESS == status)
        {
            count = 0;
     
            /* Pend until getting a response from the callback function */
            while ((LAC_RAND_NUM_CACHE_REGENERATING 
                    == lacRandCache[cacheNum].cacheState)
                   &&(count < LAC_RAND_NUM_WAIT_COUNT))
            {
                ixOsalSleep(LAC_RAND_NUM_TIMEOUT_IN_MS);
                count ++;
            }
        
            if (LAC_RAND_NUM_CACHE_REGENERATING 
                == lacRandCache[cacheNum].cacheState)
            {
                LAC_LOG_ERROR("Timeout for random Liveness callback has "
                    "expired during random number generation in cache");
                    
                status =  CPA_STATUS_FAIL;
                break;
            }
        }
        else
        {
            LAC_LOG_ERROR("Failed to generate random numbers in cache");
                
            status =  CPA_STATUS_FAIL;
            break;
        }   
    }
    
    if (CPA_STATUS_SUCCESS != status)
    {
        for(cacheNum = 0; cacheNum < LAC_RAND_NUM_CACHES; cacheNum++)
        {
            LAC_OS_CAFREE(lacRandCache[cacheNum].cacheRandData.pData,
                    lacRandCache[cacheNum].randGenOpData.lenInBytes);
        }
    }
    
    return status;
}

/**
 * Shutdown function for the Random Number Generator component
 *
 * @warning this function is not reentrant nor thread safe
 */
CpaStatus 
LacRand_Shutdown(void)
{
    Cpa8U cacheNum = 0;
 
    LAC_OS_BZERO(&lacRandStatsArr, sizeof(lacRandStatsArr));   
    
    /* Free the memory */
    for(cacheNum = 0; cacheNum < LAC_RAND_NUM_CACHES; cacheNum++)
    {
        LAC_OS_CAFREE(lacRandCache[cacheNum].cacheRandData.pData,
                lacRandCache[cacheNum].randGenOpData.lenInBytes);
    }

    return CPA_STATUS_SUCCESS;  
}

/**
 *****************************************************************************
 * @ingroup LacSym_Random
 *
 * Stats query function for the Random Number Generator component
 *
 * @warning this function is not reentrant
 *****************************************************************************/
CpaStatus
cpaCyRandQueryStats(const CpaInstanceHandle instanceHandle,
                      CpaCyRandStats *pRandStats)
{
    /* check if LAC is running */
    LAC_RUNNING_CHECK();

    if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.random.istat)
    {
        /* Used for iterating through array of statistic atomic variables */
        Cpa32U i = 0;

        LAC_CHECK_INSTANCE_HANDLE(instanceHandle);
        LAC_CHECK_NULL_PARAM(pRandStats);

        for (i = 0; i < LAC_RAND_NUM_STATS; i ++)
        {
            ((Cpa32U *)pRandStats)[i] = ixOsalAtomicGet(&lacRandStatsArr[i]);
        }
    }
    else
    {
        return CPA_STATUS_RESOURCE;
    }
    return CPA_STATUS_SUCCESS;
}

/**
 *****************************************************************************
 * @ingroup LacSym_Random
 *
 *****************************************************************************/
CpaStatus
cpaCyRandSeed(const CpaInstanceHandle instanceHandle,
        const CpaCyGenericCbFunc pRandSeedCb,
        void *callbackTag,
        const CpaCyRandSeedOpData *pSeedOpData)
{
    return CPA_STATUS_SUCCESS;
}

/**
 *****************************************************************************
 * @ingroup LacSym_Random
 *
 *****************************************************************************/
void 
LacRand_StatsShow(CpaInstanceHandle instanceHandle)
{
    CpaCyRandStats randStats = {0};
    (void)cpaCyRandQueryStats(instanceHandle, &randStats);
    
    /* Engine Info */
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
                   IX_OSAL_LOG_DEV_STDOUT,
                   SEPARATOR
                   BORDER "                Random Stats                "
                   BORDER "\n"
                   SEPARATOR,
                   0, 0, 0, 0, 0, 0);

    /* Random Info */    
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
                   IX_OSAL_LOG_DEV_STDOUT,
                   BORDER " Random Requests:                %10u " BORDER "\n"
                   BORDER " Random Completed:               %10u " BORDER "\n"
                   BORDER " Random Requests  Errors:        %10u " BORDER "\n"
                   BORDER " Random Completed Errors:        %10u " BORDER "\n"
                   SEPARATOR,
                   randStats.numRandNumRequests,
                   randStats.numRandNumCompleted,
                   randStats.numRandNumRequestErrors,
                   randStats.numRandNumCompletedErrors,
                   0, 0);   

    ixOsalLog (IX_OSAL_LOG_LVL_USER,
                   IX_OSAL_LOG_DEV_STDOUT,
                   BORDER " Random Bit Requests:            %10u " BORDER "\n"
                   BORDER " Random Bit Completed:           %10u " BORDER "\n"
                   BORDER " Random Bit Requests  Errors:    %10u " BORDER "\n"
                   BORDER " Random Bit Completed Errors:    %10u " BORDER "\n"
                   SEPARATOR,
                   randStats.numRandBitRequests,
                   randStats.numRandBitCompleted,
                   randStats.numRandBitRequestErrors,
                   randStats.numRandBitCompletedErrors,
                   0, 0);
}
