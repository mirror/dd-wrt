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
 * @file lac_buffer_desc.h
 *
 * @defgroup LacBufferDesc     Buffer Descriptors
 *
 * @ingroup LacCommon
 *
 * Functions which handle updating a user supplied buffer with the QAT
 * descriptor representation.
 *
 ***************************************************************************/


/***************************************************************************/

#ifndef LAC_BUFFER_DESC_H
#define LAC_BUFFER_DESC_H

/***************************************************************************
 * Include public/global header files
 ***************************************************************************/
#include "cpa.h"
#include "icp_buffer_desc.h"


/**
*******************************************************************************
 * @ingroup LacBufferDesc
 *      Write the buffer descriptor in QAT friendly format.
 *
 * @description
 *      Updates the Meta Data associated with the PClientList CpaBufferList
 *      This function will also return the (aligned) physical address
 *      associated with this CpaBufferList.
 *
 * @param[in] pUserBufferList            A pointer to the buffer list to 
 *                                       create the meta data for the QAT. 
 * @param[out] pBufferListAlignedPhyAddr The pointer to the aligned physical 
 *                                       address.
 *
 *****************************************************************************/
void 
LacBuffDesc_BufferListDescWrite(
    const CpaBufferList* pUserBufferList, 
    Cpa64U *pBufferListAlignedPhyAddr);


/**
*******************************************************************************
 * @ingroup LacBufferDesc
 *      Ensure the CpaBufferList is correctly formatted.
 *
 * @description
 *      Ensures the CpaBufferList pUserBufferList is correctly formatted
 *      This function will also return the total size of the buffers
 *      in the scatter gather list.
 *
 * @param[in] pUserBufferList           A pointer to the buffer list to 
 *                                      validate. 
 * @param[out] pPktSize                 The total size of the buffers in the
 *                                      scatter gather list.
 * 
 * @retval CPA_STATUS_INVALID_PARAM     BufferList failed checks
 * @retval CPA_STATUS_SUCCESS           Function executed successfully
 *
 *****************************************************************************/
CpaStatus 
LacBuffDesc_BufferListVerify(
    const CpaBufferList *pUserBufferList,
    Cpa32U *pPktSize);


/**
*******************************************************************************
 * @ingroup LacBufferDesc
 *      Get the total size of a CpaBufferList.
 *
 * @description
 *      This function returns the total size of the buffers
 *      in the scatter gather list.
 *
 * @param[in] pUserBufferList           A pointer to the buffer list to 
 *                                      calculate the total size for. 
 * @param[out] pPktSize                 The total size of the buffers in the
 *                                      scatter gather list.
 *
 *****************************************************************************/
void
LacBuffDesc_BufferListTotalSizeGet(
    const CpaBufferList *pUserBufferList, 
    Cpa32U *pPktSize);


#endif /* LAC_BUFFER_DESC_H */
