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
 * @file lac_mem_pools.c
 *       
 * @ingroup LacMemPool
 *
 * Memory Pool creation and mgmt function implementations
 *
 ***************************************************************************/

#include "cpa.h"
#include "lac_log.h"
#include "lac_mem_pools.h"
#include "lac_mem.h"
#include "lac_common.h"

#include "IxOsal.h"

#define LAC_MEM_POOLS_NUM_SUPPORTED 32
/**< @ingroup LacMemPool
 * Number of mem pools supported */

#define LAC_MEM_POOLS_NAME_SIZE 17 
/**< @ingroup LacMemPool
 * 16 bytes plus '\\0' terminator */

/**< @ingroup LacMemPool
 *     This structure is used to manage each pool created using this utility
 * feature. The client will mainatain a pointer (identifier) to the created 
 * structure per pool.
 */
typedef struct lac_mem_pool_hdr_s
{
    char poolName[LAC_MEM_POOLS_NAME_SIZE]; /*16 bytes of a pool name */
    /**< up to 16 bytes of a pool name */
    unsigned int numElementsInPool;
    /**< number of elements in the Pool */
    unsigned int blkSizeInBytes;
    /**< Block size in bytes */
    unsigned int blkAlignmentInBytes;
    /**< Block's real <aligned> size in bytes */
    unsigned int realSize;
    /**< block alignment in bytes */
    lac_mem_blk_t** trackBlks;
    /* An array of mem block pointers to track the allocated entries in pool */
    unsigned int availBlks;
    /* Number of blocks available for allocation in this pool */
    lac_mem_blk_t* pHeadPtr;
    /**< Pointer to the current head of the pool */
    lac_lock_t headLock;
    /**< Lock used for the head pointer of the pool */
}lac_mem_pool_hdr_t;

static lac_mem_pool_hdr_t* lac_mem_pools[LAC_MEM_POOLS_NUM_SUPPORTED] = {NULL};
/**< @ingroup LacMemPool
 * Array of pointers to the mem pool header structure
 */

LAC_DECLARE_HIGHEST_BIT_OF(lac_mem_blk_t);
/**< @ingroup LacMemPool
 * local constant for quickening computation of additional space allocated
 * for holding lac_mem_blk_t container-structure
 */

/**
 *******************************************************************************
 * @ingroup LacMemPool
 * This function cleans up a mem pool.
 ******************************************************************************/
static void Lac_MemPoolCleanUpInternal(lac_mem_pool_hdr_t* pPoolID);


CpaStatus
Lac_MemPoolCreate(lac_memory_pool_id_t* pPoolID, 
                  char* poolName,           
                  unsigned int numElementsInPool,   /*Number of elements*/
                  unsigned int blkSizeInBytes,      /*Block Size in bytes*/
                  unsigned int blkAlignmentInBytes, /*Block alignment (bytes)*/
                  CpaBoolean trackMemory)
{
    unsigned int poolSearch = 0;
    unsigned int counter = 0;
    lac_mem_blk_t* pMemBlkCurrent = NULL; 
    lac_mem_blk_t* pPrevBlk = NULL;

    void* pMemBlk = NULL;

    if(pPoolID == NULL)
    {
        LAC_LOG_ERROR("Invalid Pool ID param");
        return CPA_STATUS_INVALID_PARAM; /*Error*/
    }

    if(blkAlignmentInBytes & (blkAlignmentInBytes - 1))
    {
        LAC_LOG_ERROR("blkAlignmentInBytes parameter must be power of 2");
        return CPA_STATUS_INVALID_PARAM; /*Error*/
    }

    /* Find First available Pool return error otherwise */
    while(lac_mem_pools[poolSearch] != NULL)
    {
        poolSearch++;
        if(LAC_MEM_POOLS_NUM_SUPPORTED == poolSearch)
        {
            LAC_LOG_ERROR("No more memory pools available for allocation");
            return CPA_STATUS_FAIL;
        }
    }

    /* Allocate a Pool header */
    if(CPA_STATUS_SUCCESS != LAC_OS_MALLOC(&lac_mem_pools[poolSearch], 
                                        sizeof(lac_mem_pool_hdr_t)))
    {
        LAC_LOG_ERROR("Unable to allocate memory for creation of the pool");
        return CPA_STATUS_RESOURCE; /*Error*/
    }

    /* Copy in Pool Name */
    if(poolName != NULL)
    {
        strncpy(((lac_mem_pools[poolSearch])->poolName), poolName,
                  (LAC_MEM_POOLS_NAME_SIZE-1));
        /* Add '\0' termination */
       (lac_mem_pools[poolSearch])->poolName[LAC_MEM_POOLS_NAME_SIZE-1] = '\0';
    }
    else
    {
        LAC_LOG_ERROR("Invalid Pool Name pointer");
        return CPA_STATUS_INVALID_PARAM; /*Error*/
    }

    /* Allocate table for tracking memory blocks */
    if(CPA_TRUE == trackMemory)
    {
        if(CPA_STATUS_SUCCESS != 
           LAC_OS_MALLOC(&(lac_mem_pools[poolSearch]->trackBlks),
                         (sizeof(lac_mem_blk_t*) * numElementsInPool)))
        {
            LAC_LOG_ERROR(
                       "Unable to allocate memory for tracking memory blocks");
            return CPA_STATUS_RESOURCE; /*Error*/
        }
    }
    else
    {
        lac_mem_pools[poolSearch]->trackBlks = NULL;
    }

    /* Initialise locks & available blocks */
    LAC_SPINLOCK_INIT(&(lac_mem_pools[poolSearch]->headLock));
    lac_mem_pools[poolSearch]->availBlks = 0;

    /* Calculate alignment needed for allocation   */
    for(counter = 0; counter < numElementsInPool; counter++)
    {
        Cpa32U physAddr  = 0;
        /* realSize is computed for allocation of  blkSize bytes + additional 
           capacity for lac_mem_blk_t structure storage due to the some OSes 
           (BSD) limitations for memory alignment to be power of 2; 
           sizeof(lac_mem_blk_t) is being round up to the closest power of 2 - 
           optimised towards the least CPU overhead but at additional memory 
           cost
         */
    
       /* The additional size to be allocated is the greater power of 2:
        * either blkAlignmentInBytes or sozeof(lac_mem_blk_t) rounded up to
        * the closest power of 2 an immunisation against blkAlignmentInBytes
        * lesser than size of lac_mem_blk_t(ype)
        */
        Cpa32U addSize   = (blkAlignmentInBytes >= sizeof(lac_mem_blk_t) ?
                            blkAlignmentInBytes :
                            1 << (highest_bit_of_lac_mem_blk_t+1)
                           );

        Cpa32U realSize  = blkSizeInBytes + addSize;

        if(CPA_STATUS_SUCCESS != 
                LAC_OS_CAMALLOC(&pMemBlk, realSize, blkAlignmentInBytes))
        {
            Lac_MemPoolCleanUpInternal(lac_mem_pools[poolSearch]);
            LAC_LOG_ERROR("Unable to allocate contiguous chunk of memory");
            return CPA_STATUS_RESOURCE;
        }

        /* Set Pool realSize detail in the header */
        lac_mem_pools[poolSearch]->realSize = realSize;

        /* Calcaulate various addresses */
        physAddr = (Cpa32U) 
                        LAC_OS_VIRT_TO_PHYS(( ((Cpa8U *) pMemBlk) + addSize));

        /* physAddr is now already physically aligned to the greater power of 2: 
         *  blkAlignmentInBytes or sizeof(lac_mem_blk_t) round up. We safely put
         *  the structure right before the blkSize real data block.
         */
        pMemBlkCurrent = 
            (lac_mem_blk_t *) ( 
                 ((Cpa8U *) pMemBlk) + addSize - sizeof(lac_mem_blk_t));

        pMemBlkCurrent->physDataPtr = physAddr;
        pMemBlkCurrent->pMemAllocPtr = pMemBlk;
        pMemBlkCurrent->pPoolID = lac_mem_pools[poolSearch];
        pMemBlkCurrent->isInUse = CPA_FALSE;
        pMemBlkCurrent->pNext = NULL;

        if(counter == 0) /*First iteration*/
        {
            (lac_mem_pools[poolSearch])->pHeadPtr = pMemBlkCurrent;
        }

        if(pPrevBlk != NULL)
        {
            pPrevBlk->pNext = pMemBlkCurrent;
        }

        /* Store allocated memory pointer */
        if(lac_mem_pools[poolSearch]->trackBlks != NULL)
        {
            (lac_mem_pools[poolSearch]->trackBlks[counter]) = 
                (lac_mem_blk_t*)pMemBlkCurrent;
        }

        pPrevBlk = pMemBlkCurrent;
        (lac_mem_pools[poolSearch])->availBlks++;
    }

    /* Set Pool details in the header */
    (lac_mem_pools[poolSearch])->numElementsInPool   = numElementsInPool;
    (lac_mem_pools[poolSearch])->blkSizeInBytes      = blkSizeInBytes;
    (lac_mem_pools[poolSearch])->blkAlignmentInBytes = blkAlignmentInBytes;
    /* Set the Pool ID output paramter */
    *pPoolID = (unsigned long)(lac_mem_pools[poolSearch]);

    /* Success */
    return CPA_STATUS_SUCCESS;
}


void* 
Lac_MemPoolEntryAlloc(lac_memory_pool_id_t poolID)
{
    lac_mem_pool_hdr_t* pPoolID = (lac_mem_pool_hdr_t*)poolID;
    lac_mem_blk_t *pMemBlkCurrent = NULL;

#ifdef ICP_DEBUG
    /* Explicitly removing NULL PoolID check for speed */
    if(pPoolID == NULL)
    {
        LAC_LOG_ERROR("Invalid Pool ID");
        return NULL;
    }
#endif /* ICP_DEBUG */

    /* Lock accesses to the head pointer of the table */
    LAC_SPINLOCK(&(pPoolID->headLock));

    /* Remove block from pool */
    if(pPoolID->pHeadPtr != NULL)
    {
        pMemBlkCurrent = pPoolID->pHeadPtr;
        pPoolID->pHeadPtr = pPoolID->pHeadPtr->pNext;
        pPoolID->availBlks--;
    }
    else
    {
        LAC_SPINUNLOCK(&(pPoolID->headLock));
        return NULL;
    }   

    LAC_SPINUNLOCK(&(pPoolID->headLock));

    pMemBlkCurrent->isInUse = CPA_TRUE;

    return (void*)((unsigned long)(pMemBlkCurrent) + 
                   sizeof(lac_mem_blk_t));
}

void
Lac_MemPoolEntryFree(void* pEntry)
{
    lac_mem_blk_t* pMemBlk = NULL;

#ifdef ICP_DEBUG
   /* Explicitly NULL pointer check */
    if(pEntry == NULL)
    {
        LAC_LOG_ERROR("Memory Handle NULL");
        return;
    }
#endif /*ICP_DEBUG*/

    pMemBlk = (lac_mem_blk_t*)((unsigned long)pEntry - sizeof(lac_mem_blk_t));

    LAC_SPINLOCK(&(pMemBlk->pPoolID->headLock));

    pMemBlk->isInUse = CPA_FALSE;

    pMemBlk->pNext = pMemBlk->pPoolID->pHeadPtr;
    pMemBlk->pPoolID->pHeadPtr = pMemBlk;
    
    pMemBlk->pPoolID->availBlks++;

    LAC_SPINUNLOCK(&(pMemBlk->pPoolID->headLock));
}

void
Lac_MemPoolDestroy(lac_memory_pool_id_t poolID)
{
    unsigned int poolSearch = 0;
    lac_mem_pool_hdr_t* pPoolID = (lac_mem_pool_hdr_t*)poolID;

    if(pPoolID != NULL)
    {
        /*Remove entry from table*/
        while(lac_mem_pools[poolSearch] != pPoolID)
        {
            poolSearch++;
    
            if(LAC_MEM_POOLS_NUM_SUPPORTED == poolSearch)
            {
                LAC_LOG_ERROR("Invalid Pool ID submitted");
                return;
            }
        }

        lac_mem_pools[poolSearch] = NULL; /*Remove handle from pool*/

        Lac_MemPoolCleanUpInternal(pPoolID);
    }
}

static void
Lac_MemPoolCleanUpInternal(lac_mem_pool_hdr_t* pPoolID)
{
    lac_mem_blk_t* pCurrentBlk = NULL;
    void* pFreePtr = NULL;

    if(pPoolID->trackBlks == NULL)
    {
        pCurrentBlk = pPoolID->pHeadPtr;

        while(pCurrentBlk != NULL)
        {
            /* Free Data Blocks */
            pFreePtr = pCurrentBlk->pMemAllocPtr;
            pCurrentBlk = pCurrentBlk->pNext;
            LAC_OS_CAFREE(pFreePtr, pPoolID->realSize);
        }
    }
    else
    {
        Cpa32U count = 0;
        
        for(count = 0; count < pPoolID->numElementsInPool; count++)
        {
            pFreePtr = (pPoolID->trackBlks[count])->pMemAllocPtr;
            LAC_OS_CAFREE(pFreePtr, pPoolID->realSize);
        }
        LAC_OS_FREE(pPoolID->trackBlks);
    }

    if(LAC_SPINLOCK_IS_INITIALISED(&(pPoolID->headLock)))
    {
        LAC_SPINLOCK_DESTROY(&(pPoolID->headLock));
    }

    LAC_OS_FREE(pPoolID);
}

unsigned int
Lac_MemPoolAvailableEntries(lac_memory_pool_id_t poolID)
{
    lac_mem_pool_hdr_t* pPoolID = (lac_mem_pool_hdr_t*)poolID;
    if(pPoolID == NULL)
    {
        LAC_LOG_ERROR("Invalid Pool ID");
        return 0;
    }
    return pPoolID->availBlks;
}

void
Lac_MemPoolStatsShow(void)
{
    unsigned int index = 0;
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
            IX_OSAL_LOG_DEV_STDOUT,
            SEPARATOR
            BORDER "           Memory Pools Stats\n"
            SEPARATOR,
            0, 0, 0, 0, 0, 0);

    while(index < LAC_MEM_POOLS_NUM_SUPPORTED)
    {
        if(lac_mem_pools[index] != NULL)
        {
            ixOsalLog(IX_OSAL_LOG_LVL_USER,
                      IX_OSAL_LOG_DEV_STDOUT,
                      BORDER " Pool Name:             %s \n"
                      BORDER " No. Elements in Pool:  %10u \n"
                      BORDER " Element Size in Bytes: %10u \n"
                      BORDER " Alignment in Bytes:    %10u \n"
                      BORDER " No. Available Blocks:  %10u \n"
                      BORDER " Pool mem consumption:  %10u \n"
                      SEPARATOR,
                      (int) (lac_mem_pools[index]->poolName),
                      lac_mem_pools[index]->numElementsInPool,
                      lac_mem_pools[index]->blkSizeInBytes,
                      lac_mem_pools[index]->blkAlignmentInBytes,
                      lac_mem_pools[index]->availBlks,
                      (lac_mem_pools[index]->numElementsInPool *
                           lac_mem_pools[index]->realSize)
                     );
        }
        index++;
    }
}

