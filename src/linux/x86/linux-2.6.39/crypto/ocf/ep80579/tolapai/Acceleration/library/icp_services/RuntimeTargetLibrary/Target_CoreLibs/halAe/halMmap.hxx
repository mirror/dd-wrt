/**
 **************************************************************************
 * @file halMmap.hxx
 *
 * @description
 *      This file provides implementation of Ucode AE Library 
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

#ifndef __HALMMAP_HXX__
#define __HALMMAP_HXX__

#include "icptype.h"

typedef struct {
    uint64 phys;            /**< Physical address of memory region */
    uint64 virt;            /**< Virtual address of memory region */
    unsigned int size;      /**< Size of region to be mapped */
    unsigned int cacheable:1;
}HalMmapIo_T;

typedef unsigned int HalMemMapScheme;
#define MM_SCHEME_DEFAULT   0        /**< default memory map scheme */

/* addresses that will be mapped in the kernel */
typedef struct {
    HalMemMapScheme mm_scheme;       /**< Scheme for NCDRAM/CDRAM memory map */

    HalMmapIo_T dram_ch0;            /**< None-coherent DRAM*/
    HalMmapIo_T dram_ch1;            /**< Coherent DRAM */

    HalMmapIo_T sram_ch0;            /**< SRAM channel 0 */

    HalMmapIo_T cap;                 /**< All of CAP */

    HalMmapIo_T cap_ae_xfer_csr;     /**< AE Xfer CSR */
    HalMmapIo_T cap_ae_local_csr;    /**< AE Local CSR */
    HalMmapIo_T cap_pmu_csr;         /**< PMU CSR */
    HalMmapIo_T cap_global_ctl_csr;  /**< Global Control CSR */
    HalMmapIo_T cap_hash_csr;        /**< GPIO CSR */

    HalMmapIo_T scratch_rd_wr_swap;  /**< Scratch read write swap */
    HalMmapIo_T ssu_csr;             /**< SSU CSR */                 

    HalMmapIo_T eagletail_ring_csr;  /**< ET ring CSR */       
    HalMmapIo_T eagletail_ring_access; /**< ET ring put/get access */       

    HalMmapIo_T memory_target_csr;   /**< memory target CSR */       

    HalMmapIo_T ae_fastaccess_csr;   /**< AE FastAccess CSR */  

    HalMmapIo_T the_end;             /**< THE END OF TABLE */
}HalMemMap_T;

uint64 getVirXaddr(HalMemMap_T *mTab, uint64 phyXaddr, unsigned int phySize);
uint64 getPhyXaddr(HalMemMap_T *mTab, uint64 virtAddr, unsigned int virtSize);
HalMmapIo_T *getMapIo(HalMemMap_T *mTab, uint64 phyAddr, unsigned int phySize);

/* initialize memory map attribute */
#define INIT_MTAB(msec, pa, sz, va, cache)\
    msec.phys=(pa); msec.size=(sz); msec.virt=(va); \
    msec.cacheable=(cache)

/* add offset to va, unless va is zero, in that case, return 0 */
#define SADD(va, offset) (((va) != 0) ? ((va)+(offset)) : 0)

#endif /* __HALMMAP_HXX__ */
