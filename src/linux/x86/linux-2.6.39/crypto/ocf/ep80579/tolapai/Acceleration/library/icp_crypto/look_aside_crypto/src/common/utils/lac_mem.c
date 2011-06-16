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
 *****************************************************************************
 * @file lac_mem.c  Implementation of Memory Functions
 *
 * @ingroup LacMem
 *
 *****************************************************************************/

/*
*******************************************************************************
* Include public/global header files
*******************************************************************************
*/
#include "IxOsal.h"
#include "cpa.h"
#include "lac_mem.h"
#include "lac_mem_pools.h"
#include "lac_common.h"
#include "lac_common.h"
#include "lac_pke_utils.h"

/*
********************************************************************************
* Static Variables
********************************************************************************
*/

#define MAX_BUFFER_SIZE (LAC_BITS_TO_BYTES(4096))
/**< @ingroup LacMem
 * Maximum size of the buffers used in the align function */

static lac_memory_pool_id_t lac_align_pool = LAC_MEM_POOL_INIT_POOL_ID;
/**< @ingroup LacMem
 * Identifier for pool of aligned buffers */

/*
*******************************************************************************
* Define public/global function definitions
*******************************************************************************
*/

/**
 * @ingroup LacMem
 */
CpaStatus
LacMem_InitBuffers(Cpa64U numAsymConcurrentReq)
{
    Cpa64U actualNumAsymBuffersReq = 0;

    /* Several buffers may be required per request per direction */
    actualNumAsymBuffersReq = (numAsymConcurrentReq
                                * (LAC_PKE_BUFFERS_PER_OP_MAX
                                * LAC_PKE_MAX_OUTSTANDING_REQUESTS));


    return Lac_MemPoolCreate(&lac_align_pool, "ALIGN BUFFERS",
                             actualNumAsymBuffersReq, MAX_BUFFER_SIZE, 
                             LAC_64BYTE_ALIGNMENT, CPA_FALSE);
}

/**
 * @ingroup LacMem
 */
void
LacMem_DestroyBuffers(void)
{
    Lac_MemPoolDestroy(lac_align_pool);
}

/**
 * @ingroup LacMem
 */
Cpa8U *
LacMem_BufferAlign(Cpa8U *pUserBuffer,
                   Cpa32U userLen,
                   Cpa32U workingLen,
                   icp_lac_mem_padding_t padding)
{
    Cpa64U userPhysAddr = 0;
    Cpa8U *pWorkingBuffer = NULL;
    Cpa32U padSize = 0;

    if ((userLen > 0) && (NULL == pUserBuffer))
    {
        LAC_LOG_ERROR("pUserBuffer parameter is NULL");
        return NULL;
    }


    /* shouldn't trim the user buffer */
    if (workingLen < userLen)
    {
        LAC_LOG_ERROR2("Cannot trim input buffer from %u to %u", 
                       userLen, workingLen);
        return NULL;
    }

    if (NULL != pUserBuffer)
    {
        userPhysAddr = LAC_MEM_CAST_PTR_TO_UINT64(
                                         LAC_OS_VIRT_TO_PHYS(pUserBuffer));
    }

    padSize = workingLen - userLen;

    /* check 8-byte alignment or size */
    if (((Cpa32U)userPhysAddr & 0x7) || (padSize > 0))
    {
        pWorkingBuffer = (Cpa8U *)Lac_MemPoolEntryAlloc(lac_align_pool);
        if (NULL == pWorkingBuffer)
        {
            LAC_LOG_ERROR("failed to allocate pWorkingBuffer");
            return NULL;
        }
        IX_OSAL_ASSERT(pWorkingBuffer != NULL);

        LAC_OS_BZERO(pWorkingBuffer, workingLen);

        if (padding != ICP_LAC_MEM_NO_COPY)
        {
            if (padding & ICP_LAC_MEM_PAD_LEFT)
            {
                if (userLen)
                {
                    memcpy(pWorkingBuffer + padSize, pUserBuffer, userLen);
                }
                if ((padding & ICP_LAC_MEM_PAD_ZEROES) && (padSize != 0))
                {
                    LAC_OS_BZERO(pWorkingBuffer, padSize);
                }
            }
            else if (padding & ICP_LAC_MEM_PAD_RIGHT)
            {
                if (userLen)
                {
                    memcpy(pWorkingBuffer, pUserBuffer, userLen);
                }
                if ((padding & ICP_LAC_MEM_PAD_ZEROES) && (padSize != 0))
                {
                    LAC_OS_BZERO(pWorkingBuffer + userLen, padSize);
                }
            }
            else
            {
                /* can't parse padding */
                LAC_LOG_ERROR1("Unknown padding 0x%x specified", padding);
                return NULL;
            }
        } /* if (padding != ICP_LAC_MEM_NO_COPY) ... */

        return pWorkingBuffer;
    } /* if ((userPhysAddr & 0x7) || (padSize > 0)) ... */

    return pUserBuffer;
}

/**
 * @ingroup LacMem
 */
CpaStatus
LacMem_BufferRestore(Cpa8U *pUserBuffer,
                     Cpa32U userLen,
                     Cpa8U *pWorkingBuffer,
                     Cpa32U workingLen,
                     icp_lac_mem_padding_t padding)
{
    Cpa32U padSize;

    /* NULL is a valid value for working buffer as this function may be
     * called to clean up in an error case where all the align operations
     * were not completed */
    if (NULL == pWorkingBuffer)
    {
        return CPA_STATUS_SUCCESS;
    }

    if (workingLen < userLen)
    {
        LAC_LOG_ERROR("Invalid buffer sizes");
        return CPA_STATUS_INVALID_PARAM;
    }

    if (pUserBuffer != pWorkingBuffer)
    {
        padSize = workingLen - userLen;

        if (padding != ICP_LAC_MEM_NO_COPY)
        {
            if (padding == ICP_LAC_MEM_PAD_LEFT)
            {
                memcpy(pUserBuffer, pWorkingBuffer + padSize, userLen);
            }
            else if (padding == ICP_LAC_MEM_PAD_RIGHT)
            {
                memcpy(pUserBuffer, pWorkingBuffer, userLen);
            }
            else
            {
                /* can't parse padding */
                LAC_LOG_ERROR1("Unknown padding 0x%x specified", padding);
                return CPA_STATUS_INVALID_PARAM;
            }
        }
        
        Lac_MemPoolEntryFree(pWorkingBuffer);
    }
    return CPA_STATUS_SUCCESS;
}
