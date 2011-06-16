/**
 **************************************************************************
 * @file hal_scratch.h
 *
 * @description
 *      This is the header file for Hardware Abstraction Layer -- Scratchpad Memory Unit
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
 * @file hal_scratch.h
 * 
 * @ingroup icp_hal
 *
 * @description
 *      This header file contains the macros of SCRATCH access
 *
 *****************************************************************************/

#ifndef __HAL_SCRATCH_H
#define __HAL_SCRATCH_H

#include "core_io.h"
#include "halMmap.h"

#define SCRATCH_SIZE       0x4000         /**< SCRATCH memory size */
#define SCRATCH_CONFIG_CSR 0x800          /**< SCRATCH CSR offset */ 

/* internal macros to access SCRATCH CSR */
#define SCRATCH_CSR(csr, ring) ((Hal_cap_global_ctl_csr_virtAddr + SCRATCH_CONFIG_CSR + ((csr) + ((0xf & (ring)) << 4))))
#define SET_SCRATCH_CSR(csr, ring, val) WRITE_LWORD(SCRATCH_CSR((csr),(ring)), (val))
#define GET_SCRATCH_CSR(csr, ring) READ_LWORD(SCRATCH_CSR((csr), (ring)))

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Writes a longword data to SCRATCH location.
 *      
 * @param addr - IN Specifies SCRATCH address offset
 * @param val - IN Specifies value to write
 *
 * @retval - none
 * 
 * 
 *****************************************************************************/

#define SCRATCH_WRITE(addr, val) WRITE_LWORD((addr) + Hal_scratch_rd_wr_swap_virtAddr, (val))

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Reads a longword data from SCRATCH location.
 *      
 * @param addr - IN Specified SRAM SCRATCH offset
 *
 * @retval - read data
 * 
 * 
 *****************************************************************************/

#define SCRATCH_READ(addr) READ_LWORD((addr) + Hal_scratch_rd_wr_swap_virtAddr)

#endif      /* __HAL_SCRATCH_H */
