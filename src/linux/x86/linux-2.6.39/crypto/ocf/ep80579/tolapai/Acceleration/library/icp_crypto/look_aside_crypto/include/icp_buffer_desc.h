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
 * @file icp_buffer_desc.h
 * 
 * @defgroup icp_BufferDesc Buffer descriptor for LAC
 * 
 * @ingroup LacCommon
 * 
 * @description
 *      This file contains details of the hardware buffer descriptors used to 
 *      communicate with the QAT.
 * 
 *****************************************************************************/
#ifndef ICP_BUFFER_DESC_H
#define ICP_BUFFER_DESC_H

#include "cpa.h"

typedef Cpa64U  icp_qat_addr_width_t;    // hi32 first, lo32 second

// Alignement constraint of the buffer list. 
#define ICP_DESCRIPTOR_ALIGNMENT_BYTES  8

/**
 *****************************************************************************
 * @ingroup icp_BufferDesc
 *      Buffer descriptors for FlatBuffers - used in communications with
 *      the QAT.
 * 
 * @description
 *      A QAT friendly buffer descriptor.
 *      All buffer descriptor described in this structure are physcial
 *      and are 64 bit wide.
 *
 *      Updates in the CpaFlatBuffer should be also reflected in this
 *      structure
 *
 *****************************************************************************/
typedef struct icp_flat_buffer_desc_s 
{
    Cpa32U               dataLenInBytes;
    Cpa32U               reserved;
    icp_qat_addr_width_t phyBuffer;
    /**< The client will allocate memory for this using API function calls
      *  and the access layer will fill it and the QAT will read it.
      */
} icp_flat_buffer_desc_t;


/**
 *****************************************************************************
 * @ingroup icp_BufferDesc
 *      Buffer descriptors for BuffersLists - used in communications with
 *      the QAT.
 * 
 * @description
 *      A QAT friendly buffer descriptor.
 *      All buffer descriptor described in this structure are physcial
 *      and are 64 bit wide.
 *
 *      Updates in the CpaBufferList should be also reflected in this structure
 *
 *****************************************************************************/
typedef struct icp_buffer_list_desc_s 
{
    Cpa32U                  numBuffers;
    Cpa32U                  reserved;
    icp_flat_buffer_desc_t  phyBuffers[];
    /**< Unbounded array of physical buffer pointers, these point to the
      *  FlatBufferDescs. The client will allocate memory for this using
      *  API function calls and the access layer will fill it and the QAT
      *  will read it.
      */
} icp_buffer_list_desc_t;

#endif /* ICP_BUFFER_DESC_H */
