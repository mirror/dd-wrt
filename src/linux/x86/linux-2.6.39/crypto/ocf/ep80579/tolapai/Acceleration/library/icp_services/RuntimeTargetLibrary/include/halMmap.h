/**
 **************************************************************************
 * @file halMmap.h
 *
 * @description
 *      This is the header file for Hardware Abstraction Layer
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
 * @file halMmap.h
 * 
 * @ingroup icp_hal
 *
 * @description
 *      This header file that contains the definitons of exported virtual 
 *      address symbols.
 *
 *****************************************************************************/

#ifndef __HALMMAP_H__
#define __HALMMAP_H__

#include "icptype.h"
#include "core_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Virtual address pointer to various memory regions.
   These values are initialized by the HAL library. */
EXTERN_API volatile uint64 Hal_dram_ch0_virtAddr;           /**< DRAM chanel0 (NCDRAM) */
EXTERN_API volatile uint64 Hal_dram_ch1_virtAddr;           /**< DRAM chanel1 (CDRAM) */

EXTERN_API volatile uint64 Hal_sram_ch0_virtAddr;           /**< SRAM channel0 -- beg thru end of read-write region */
EXTERN_API volatile uint64 Hal_sram_ch0_rd_wr_virtAddr;     /**< SRAM channel0 read/write */

EXTERN_API volatile uint64 Hal_cap_global_ctl_csr_virtAddr; /**< Global Control CSR */
EXTERN_API volatile uint64 Hal_cap_ae_xfer_csr_virtAddr;    /**< AE Xfer CSR */
EXTERN_API volatile uint64 Hal_cap_ae_local_csr_virtAddr;   /**< AE local CSR */
EXTERN_API volatile uint64 Hal_cap_pmu_csr_virtAddr;        /**< PMU CSR */
EXTERN_API volatile uint64 Hal_cap_hash_csr_virtAddr;       /**< hash CSR */

EXTERN_API volatile uint64 Hal_scratch_rd_wr_swap_virtAddr; /**< Scratch read write swap */

EXTERN_API volatile uint64 Hal_ae_fastaccess_csr_virtAddr;  /**< fast access CSR */
EXTERN_API volatile uint64 Hal_ssu_csr_virtAddr;            /**< SSU CSR */ 

EXTERN_API volatile uint64 Hal_eagletail_ring_csr_virtAddr;    /**< ET ring CSR */ 
EXTERN_API volatile uint64 Hal_eagletail_ring_access_virtAddr; /**< ET ring put/get access */ 

EXTERN_API volatile uint64 Hal_memory_target_csr_virtAddr;     /**< memory target CSR */ 

#ifdef __cplusplus
}
#endif


#endif      /* __HALMMAP_H__ */
