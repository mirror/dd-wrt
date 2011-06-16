/**
 **************************************************************************
 * @file hal_et_ring.h
 *
 * @description
 *      This is the header file for Hardware Abstraction Layer -- Eagletail Ring Unit
 *
 * @par 
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
 **************************************************************************/

/**
 *****************************************************************************
 * @file hal_et_ring.h
 * 
 * @ingroup icp_hal
 *
 * @description
 *      This header file contains the macros accessing eagletail ring.
 *
 *****************************************************************************/

#ifndef __HAL_ET_RING_H
#define __HAL_ET_RING_H

#include "halAeApi.h"

#define MAX_EAGLETAIL_RING_ENTRIES    (64U)     /**< max eagletail ring entries */

/* raw macros to access ring CSR */

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Reads a longword data from eagletail ring control and status register.
 *      
 * @param arg_CSR - IN Specifies register offset
 *
 * @retval - read data
 * 
 * 
 *****************************************************************************/

#define EAGLETAIL_RING_CSR_READ_RAW(arg_CSR) \
            READ_LWORD((Hal_eagletail_ring_csr_virtAddr + (unsigned int)arg_CSR))

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Writes a longword data to eagletail ring control and status register.
 *      
 * @param arg_CSR - IN Specifies register offset
 * @param arg_Data - IN A longword data to write
 *
 * @retval - none
 * 
 * 
 *****************************************************************************/

#define EAGLETAIL_RING_CSR_WRITE_RAW(arg_CSR, arg_Data) \
            WRITE_LWORD((Hal_eagletail_ring_csr_virtAddr + (unsigned int)arg_CSR),  (unsigned int)(arg_Data))

/* macros to access ring number specific CSR */

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Reads a longword data from eagletail ring control and status register
 *      specified by ring number and address offset.
 *      
 * @param arg_RingID - IN Specifies ring number
 * @param arg_CSR - IN Specifies register offset
 *
 * @retval - read data
 * 
 * 
 *****************************************************************************/

#define EAGLETAIL_RING_CSR_READ(arg_RingID, arg_CSR) \
            READ_LWORD((Hal_eagletail_ring_csr_virtAddr + (unsigned int)arg_CSR + ((unsigned int)(arg_RingID) << 2U)))

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Writes a longword data to eagletail ring control and status register
 *      specified by ring number and address offset.
 *      
 * @param arg_RingID - IN Specifies ring number
 * @param arg_CSR - IN Specifies register offset
 * @param arg_Data - IN A longword data to write
 *
 * @retval - none
 * 
 * 
 *****************************************************************************/

#define EAGLETAIL_RING_CSR_WRITE(arg_RingID, arg_CSR, arg_Data) \
            WRITE_LWORD((Hal_eagletail_ring_csr_virtAddr + (unsigned int)arg_CSR + ((unsigned int)(arg_RingID) << 2U)), \
                        (unsigned int)(arg_Data))

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Puts a longword data to specified eagletail ring using MMIO access mode
 *      
 * @param arg_RingID - IN Specifies ring number
 * @param arg_Data - IN A longword data to put to the ring
 *
 * @retval - none
 * 
 * 
 *****************************************************************************/

#define EAGLETAIL_RING_PUT(arg_RingID, arg_Data) \
            WRITE_LWORD((Hal_eagletail_ring_access_virtAddr + ((unsigned int)(arg_RingID) << 3U)), \
                        (unsigned int)(arg_Data))

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Gets a longword data from specified eagletail ring using MMIO access mode
 *      
 * @param arg_RingID - IN Specifies ring number
 * @param arg_Data - IN A longword data to put to the ring
 *
 * @retval - get data
 * 
 * 
 *****************************************************************************/

#define EAGLETAIL_RING_GET(arg_RingID) \
            READ_LWORD((Hal_eagletail_ring_access_virtAddr + ((unsigned int)(arg_RingID) << 3U)))

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Check if specified eagletail ring is full or not.
 *      
 * @param arg_RingID - IN Specifies ring number
 *
 * @retval 1 ring full
 * @retval 0 ring not full
 * 
 * 
 *****************************************************************************/
                        
#define EAGLETAIL_RING_IS_FULL_STATUS(arg_RingID) \
            ((arg_RingID < 32) ? \
             (READ_LWORD(Hal_eagletail_ring_csr_virtAddr + ET_RING_F_STAT_0) & (1U << (unsigned int)(arg_RingID))) : \
             (READ_LWORD(Hal_eagletail_ring_csr_virtAddr + ET_RING_F_STAT_1) & (1U << (unsigned int)(arg_RingID - 32))))

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Check if specified eagletail ring is empty or not.
 *      
 * @param arg_RingID - IN Specifies ring number
 *
 * @retval 1 ring empty
 * @retval 0 ring not empty
 * 
 * 
 *****************************************************************************/
            
#define EAGLETAIL_RING_IS_EMPTY_STATUS(arg_RingID) \
            ((arg_RingID < 32) ? \
             (READ_LWORD(Hal_eagletail_ring_csr_virtAddr + ET_RING_E_STAT_0) & (1U << (unsigned int)(arg_RingID))) : \
             (READ_LWORD(Hal_eagletail_ring_csr_virtAddr + ET_RING_E_STAT_1) & (1U << (unsigned int)(arg_RingID - 32))))

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Check if specified eagletail ring is near full or not.
 *      
 * @param arg_RingID - IN Specifies ring number
 *
 * @retval 1 ring near full
 * @retval 0 ring not near full
 * 
 * 
 *****************************************************************************/

#define EAGLETAIL_RING_IS_NEAR_FULL_STATUS(arg_RingID) \
            ((arg_RingID < 32) ? \
             (READ_LWORD(Hal_eagletail_ring_csr_virtAddr + ET_RING_NF_STAT_0) & (1U << (unsigned int)(arg_RingID))) : \
             (READ_LWORD(Hal_eagletail_ring_csr_virtAddr + ET_RING_NF_STAT_1) & (1U << (unsigned int)(arg_RingID - 32))))

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Check if specified eagletail ring is near empty or not.
 *      
 * @param arg_RingID - IN Specifies ring number
 *
 * @retval 1 ring near empty
 * @retval 0 ring not near empty
 * 
 * 
 *****************************************************************************/
            
#define EAGLETAIL_RING_IS_NEAR_EMPTY_STATUS(arg_RingID) \
            ((arg_RingID < 32) ? \
             (READ_LWORD(Hal_eagletail_ring_csr_virtAddr + ET_RING_NE_STAT_0) & (1U << (unsigned int)(arg_RingID))) : \
             (READ_LWORD(Hal_eagletail_ring_csr_virtAddr + ET_RING_NE_STAT_1) & (1U << (unsigned int)(arg_RingID - 32))))

#endif      /* __HAL_ET_RING_H */

