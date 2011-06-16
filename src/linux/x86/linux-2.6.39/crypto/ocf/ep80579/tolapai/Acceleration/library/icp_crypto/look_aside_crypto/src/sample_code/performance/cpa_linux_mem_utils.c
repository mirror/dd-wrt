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
 * @file cpa_linux_mem_utils.c
 *
 * @defgroup sampleUtilsPerf  linux memory allocation and free Utility
 * functions for Performance code 
 *
 * @ingroup perfCode
 *
 * @description
 *      This file contains functions for allocating and freeing 
 *      aligned memory in linux.
 *
 *****************************************************************************/

/******************************************************************************
 * 
 *      Functions contained in this file:
 *        - sampleCreateBuffers
 *        - sampleFreeBuffers
 *        - allocAlignedMem
 *        - freeAlignedMem
 *        - allocOsMemCheck
 *
 *****************************************************************************/


    #include <linux/io.h>

#include "cpa.h"
#include "cpa_perf_defines.h"

#include "cpa_cy_common.h"
#include "cpa_cy_sym.h"
#include "cpa_cy_rand.h"
#include "cpa_cy_im.h"


 /*****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      Function for pre-allocating Buffer List for perform operations
 *
 *****************************************************************************/

CpaStatus sampleCreateBuffers( const Cpa32U packetSizeInBytes,
                            CpaFlatBuffer *pFlatBuffArray[],
                            CpaBufferList *pBuffListArray[])
{
    CpaStatus      status          = CPA_STATUS_SUCCESS;
    CpaFlatBuffer  *pTempFlatBuff  = NULL;
    CpaBufferList  *pTempBufList   = NULL;
    Cpa32U         bufferMetaSize  = 0;
    Cpa32U         bufferListSize  = 0;
    Cpa32U         createCount     = 0;
    Cpa8U          *pBufferMeta    = NULL;

    /*
     * calculate memory size which is required for pPrivateMetaData
     * member of CpaBufferList
     */
    status = cpaCyBufferListGetMetaSize( CPA_INSTANCE_HANDLE_SINGLE,
                NUM_UNCHAINED_BUFFERS, &bufferMetaSize);

    bufferListSize = LAC_BUFFER_LIST_SIZE_GET(NUM_UNCHAINED_BUFFERS);

    for (createCount = 0;
         createCount < NUM_PRE_ALLOCATED_BUFF_LISTS;
         createCount++)
    {
        /* Allocate memory for temp flat buffer */
        pTempFlatBuff = (CpaFlatBuffer*)
                            allocOsMemCheck(sizeof(CpaFlatBuffer), &status);

        if(CPA_STATUS_SUCCESS == status)
        {
            /* Allocate memory for pPrivateMetaData */
            pBufferMeta = (Cpa8U*)allocOsMemCheck(bufferMetaSize, &status);

            if(CPA_STATUS_SUCCESS != status)  
            {
                PRINT_ERR("Failed to allocate pBufferMeta memory\n");
                PRINT_ERR("Allocation failed on instance %d\n", createCount);
                return  CPA_STATUS_FAIL;
            }
        }
        else
        {
            return status;
        }

        if(CPA_STATUS_SUCCESS == status)
        {
            /* Allocate memory for buffer list structure */

            pTempBufList = (CpaBufferList*)
                               allocOsMemCheck(bufferListSize, &status);

            if(CPA_STATUS_SUCCESS != status)   
            {
                PRINT_ERR("Failed to allocate bufferlist memory\n");
                PRINT_ERR("Allocation failed on instance %d\n", createCount);
                return CPA_STATUS_FAIL;
            }
        }
        /* Allocate aligned memory for specified packet size */
        pTempFlatBuff->pData = (uint8_t *)allocAlignedMem(packetSizeInBytes,
                BYTE_ALIGNMENT_8); 

        if(NULL == pTempFlatBuff->pData)
        {
            PRINT_ERR("Failed to allocate packetSizeData memory\n");
            PRINT_ERR("Allocation failed on instance %d\n", createCount);
            return CPA_STATUS_FAIL;
        }

        pTempFlatBuff->dataLenInBytes = packetSizeInBytes;

        if(NULL != pTempBufList)
        {
            /*
             * Fill in elements of buffer list struct.
             * For this scenario- each buffer list only
             * contains one buffer
             */
            pTempBufList->numBuffers         = NUM_UNCHAINED_BUFFERS;
            pTempBufList->pPrivateMetaData   = pBufferMeta;

            /* set up the pBuffers pointer */
            pTempBufList->pBuffers           = pTempFlatBuff;
        }

        if(CPA_STATUS_SUCCESS != sampleRandGenPerform(pTempFlatBuff->pData,
                    packetSizeInBytes))
        {
            PRINT_ERR("Failed to create random data\n");
            return CPA_STATUS_FAIL;
        }
        
        /*
         * Setup up pointers in flat buffer array and buffer list array
         * continue loop and increment to the next position
         */
        
        pFlatBuffArray[createCount] = pTempFlatBuff;
        pBuffListArray[createCount] = pTempBufList;
    } /* end of pre allocated buffer for loop */

    /* 
     * Return CPA_STATUS_SUCCESS if all buffers have
     * been correctly allocated
     */
    return CPA_STATUS_SUCCESS; 
}

/**
 *****************************************************************************
 * @ingroup perfCode
 *
 * @description
 *      Function for cleaning up memory allocated during call to create
 *      buffers function
 *
 *****************************************************************************/

void sampleFreeBuffers( const Cpa32U packetSizeInBytes,
                         CpaFlatBuffer *srcBuffPtrArray[],
                         CpaBufferList *srcBuffListArray[])
{
    Cpa32U      freeMemCount     = 0;

    /*
     * Loop through and free all buffers that have been
     * pre-allocated.
     */
    for( freeMemCount = 0; freeMemCount < NUM_PRE_ALLOCATED_BUFF_LISTS;
          freeMemCount++)
    {
        if( ( NULL == srcBuffPtrArray[freeMemCount]  ) ||
            ( NULL == srcBuffListArray[freeMemCount] ) )
        {
            return;
        }

        if(NULL != srcBuffPtrArray[freeMemCount]->pData)
        {
            OS_MEM_ALIGNED_FREE((void *)srcBuffPtrArray[freeMemCount]->pData);
        }

        if(NULL != srcBuffPtrArray[freeMemCount])
        {
            OS_MEM_FREE_CHECK(srcBuffPtrArray[freeMemCount]);
        }

        if(NULL != srcBuffListArray[freeMemCount]->pBuffers)
        {
            OS_MEM_FREE_CHECK(srcBuffListArray[freeMemCount]->pPrivateMetaData);
        }

        if(NULL != srcBuffListArray[freeMemCount])
        {
            OS_MEM_FREE_CHECK(srcBuffListArray[freeMemCount]);
        }
    }

    return;
}


/*
 *  The function allocates a chunck of memory bigger that the required size
 *  It then calculate the aligned ptr that should be returned to the user.
 *  In the memory just above the returned chunck, the funcion stores the
 *  structure with the memory information
 *
 *  +---+-------------------------+------------------------------- +---+
 *  |xxx| memAllocInfoStruct      | memory returned to user (size) |xxx|
 *  +---+-------------------------+--------------------------------+---+
 *  ^                             ^
 *  mAllocMemPtr                  Ptr returned to the caller of MemAlloc
 *
 */

void *allocAlignedMem(Cpa32U size, Cpa32U alignment)   
{
    void*  ptr       = NULL;
    void*  pRet      = NULL;
    Cpa32U toPadSize = 0;
    Cpa32U padding   = 0;
    Cpa32U alignment_offset = 0;
    memAllocInfoStruct_t memInfo = {0};

    if (sizeof(memAllocInfoStruct_t) > alignment)
    {
        toPadSize = sizeof(memAllocInfoStruct_t) + alignment;
        padding = MEM_PADDING(toPadSize, alignment);
    }
    else
    {
        toPadSize = alignment;
        padding = 0;
    }

    memInfo.mSize = size + toPadSize + padding;

    if (memInfo.mSize > MAX_KMALLOC_MEM)
    {
        return NULL;
    }

    ptr = kmalloc (memInfo.mSize, GFP_KERNEL);

    if (NULL == ptr)
    {
        PRINT_ERR("allocAlignedMem allocation failed\n");
        return NULL;
    }

    alignment_offset = (Cpa32U)ptr % alignment;
    memInfo.mAllocMemPtr = ptr;

    pRet = (void*) ((Cpa8U*)ptr + toPadSize + padding + alignment_offset);
    memcpy((void*)((Cpa8U*)pRet - sizeof(memAllocInfoStruct_t)),
           (void*)(&memInfo),
           sizeof(memAllocInfoStruct_t));

    return pRet;
}


void freeAlignedMem(void *ptr)
{
    memAllocInfoStruct_t *pMemInfo = NULL;

    pMemInfo = (memAllocInfoStruct_t *)((Cpa8U *)ptr - 
                                  sizeof(memAllocInfoStruct_t));

    if (pMemInfo->mSize == 0 || pMemInfo->mAllocMemPtr == NULL)
    {
        PRINT_ERR("freeAlignedMem: free aligned memory failed\n");
        return;
    }
    
    kfree (pMemInfo->mAllocMemPtr);
}


void *allocOsMemCheck(Cpa32U memLenBytes, CpaStatus *status)
{
    void *pMem = kmalloc(memLenBytes, GFP_KERNEL);
    if(NULL == pMem)
    {
        PRINT_ERR("ERROR: FAILED TO ALLOC OS MEMORY\n");
        *status = CPA_STATUS_FAIL;
    }
    else
    {
        *status = CPA_STATUS_SUCCESS;
    }

    return pMem;
}

