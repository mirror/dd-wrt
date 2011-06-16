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
 * @file lac_buffer_desc.c  Utility functions for setting buffer descriptors
 *
 * @ingroup LacBufferDesc
 *
 *****************************************************************************/

/*
*******************************************************************************
* Include public/global header files
*******************************************************************************
*/
#include "lac_buffer_desc.h"
#include "lac_mem.h"
#include "cpa_cy_common.h"


/*
*******************************************************************************
* Define public/global function definitions
*******************************************************************************
*/

void 
LacBuffDesc_BufferListDescWrite(const CpaBufferList *pClientList, 
                                Cpa64U *pBufListAlignedPhyAddr)
{
    Cpa32U numBuffers = pClientList->numBuffers;
    icp_qat_addr_width_t bufListDescPhyAddr = 0;
    icp_qat_addr_width_t bufListAlignedPhyAddr = 0;
    CpaFlatBuffer *pCurrClientFlatBuffer = pClientList->pBuffers;
    icp_buffer_list_desc_t *pBufferListDesc = NULL;
    icp_flat_buffer_desc_t *pCurrFlatBufDesc = NULL;
    
    LAC_ENSURE_NOT_NULL(pClientList);
    LAC_ENSURE_NOT_NULL(pBufListAlignedPhyAddr);
    /*
     * Get the physical address of this descriptor - need to offset by the 
     * alignment restrictions on the buffer descriptors
     */
    bufListDescPhyAddr = (icp_qat_addr_width_t)(unsigned long)
                          LAC_OS_VIRT_TO_PHYS(pClientList->pPrivateMetaData);

    bufListAlignedPhyAddr = LAC_ALIGN_POW2_ROUNDUP(bufListDescPhyAddr, 
                                    ICP_DESCRIPTOR_ALIGNMENT_BYTES);
   
    /* Increment the virtual pointer to correspond to the aligned physical
     * address */ 
    pBufferListDesc = (icp_buffer_list_desc_t *) ((Cpa8U *)
                    pClientList->pPrivateMetaData
                    + (bufListAlignedPhyAddr - bufListDescPhyAddr));
    
    /* Go past the Buffer List descriptor to the list of buffer descriptors */
    pCurrFlatBufDesc = (icp_flat_buffer_desc_t *)(
                            (pBufferListDesc->phyBuffers));
    
    pBufferListDesc->numBuffers = numBuffers;
    
    if(numBuffers == 0)
    {
        /* In the case where there are zero buffers within the BufList
         * it is required by firmware that the number is set to 1 
         * but the phyBuffer and dataLenInBytes are set to NULL.*/
         pBufferListDesc->numBuffers = 1;
         pCurrFlatBufDesc->dataLenInBytes = 0;
         pCurrFlatBufDesc->phyBuffer = LAC_MEM_CAST_PTR_TO_UINT64(NULL);
    }
    
    while (0 != numBuffers)
    {
        pCurrFlatBufDesc->dataLenInBytes = 
            pCurrClientFlatBuffer->dataLenInBytes; 
        
        pCurrFlatBufDesc->phyBuffer = LAC_MEM_CAST_PTR_TO_UINT64(
                LAC_OS_VIRT_TO_PHYS(pCurrClientFlatBuffer->pData)); 

        pCurrFlatBufDesc++;
        pCurrClientFlatBuffer++;

        numBuffers--;
    }
    
    *pBufListAlignedPhyAddr = bufListAlignedPhyAddr;
}

CpaStatus
LacBuffDesc_BufferListVerify(const CpaBufferList *pClientBufferList, 
                             Cpa32U *pPktSize)
{
    CpaFlatBuffer *pCurrClientFlatBuffer = NULL;
    Cpa32U numBuffers = 0;
   
    LAC_CHECK_NULL_PARAM(pClientBufferList->pPrivateMetaData);
    LAC_CHECK_NULL_PARAM(pClientBufferList->pBuffers);
    LAC_ENSURE_NOT_NULL(pPktSize);    

    numBuffers = pClientBufferList->numBuffers;

    if (0 == pClientBufferList->numBuffers)
    {
        LAC_INVALID_PARAM_LOG("Number of Buffers");
        return CPA_STATUS_INVALID_PARAM;
    }

    pCurrClientFlatBuffer = pClientBufferList->pBuffers;

    *pPktSize = 0;
    while (0 != numBuffers)
    {
        if ((NULL == pCurrClientFlatBuffer->pData) || 
            (0 == pCurrClientFlatBuffer->dataLenInBytes))
        {
            LAC_INVALID_PARAM_LOG1("FlatBufferList Element %d", 
                pClientBufferList->numBuffers - numBuffers);

            return CPA_STATUS_INVALID_PARAM;
        }

        *pPktSize += pCurrClientFlatBuffer->dataLenInBytes;
        pCurrClientFlatBuffer++;

        numBuffers--;
    }

    return CPA_STATUS_SUCCESS;
}

/**
 ******************************************************************************
 * @ingroup LacBufferDesc
 *****************************************************************************/
void
LacBuffDesc_BufferListTotalSizeGet(
    const CpaBufferList *pClientBufferList, 
    Cpa32U *pPktSize)
{
    CpaFlatBuffer *pCurrClientFlatBuffer = pClientBufferList->pBuffers;
    Cpa32U numBuffers = pClientBufferList->numBuffers;

    LAC_ENSURE_NOT_NULL(pClientBufferList);
    LAC_ENSURE_NOT_NULL(pPktSize);

    *pPktSize = 0;
    while (0 != numBuffers)
    {
        *pPktSize += pCurrClientFlatBuffer->dataLenInBytes;
        pCurrClientFlatBuffer++;
        numBuffers--;
    }
}

/**
 ******************************************************************************
 * @ingroup LacBufferDesc
 *****************************************************************************/
CpaStatus
cpaCyBufferListGetMetaSize(const CpaInstanceHandle instanceHandle,
        Cpa32U numBuffers,
        Cpa32U *pSizeInBytes)
{
#ifdef ICP_PARAM_CHECK
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);
    LAC_CHECK_NULL_PARAM(pSizeInBytes);
    
#endif

    /* In the case of zero buffers we still need to allocate one
     * descriptor to pass to the firmware */
    if(0 == numBuffers)
    {
        numBuffers = 1;
    }

    /* Note: icp_buffer_list_desc_t is 8 bytes in size and
     * icp_flat_buffer_desc_t is 16 bytes in size. Therefore if
     * icp_buffer_list_desc_t is aligned so will each
     * icp_flat_buffer_desc_t structure */
    
    *pSizeInBytes =  sizeof(icp_buffer_list_desc_t) + 
                     (sizeof(icp_flat_buffer_desc_t) * numBuffers) + 
                     ICP_DESCRIPTOR_ALIGNMENT_BYTES;

    return CPA_STATUS_SUCCESS;
}
