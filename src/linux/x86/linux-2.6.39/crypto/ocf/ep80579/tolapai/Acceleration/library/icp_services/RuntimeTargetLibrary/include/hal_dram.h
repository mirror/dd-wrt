/**
 **************************************************************************
 * @file hal_dram.h
 *
 * @description
 *      This is the header file for Hardware Abstraction Layer -- DRAM Unit
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
 * @file hal_dram.h
 * 
 * @ingroup icp_hal
 *
 * @description
 *      This header file contains the macros of DRAM access
 *
 *****************************************************************************/

#ifndef __HAL_DRAM_H
#define __HAL_DRAM_H

#include "core_io.h"
#include "halMmap.h"

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Writes a longword data to non-coherent DRAM location.
 *      
 * @param addr - IN Specifies non-coherent dram address offset
 * @param val - IN Specifies value to write
 *
 * @retval - none
 * 
 * 
 *****************************************************************************/

#define DRAM_WRITE(addr, val) WRITE_LWORD(((addr)) + Hal_dram_ch0_virtAddr, (val))

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Reads a longword data from non-coherent DRAM location.
 *      
 * @param addr - IN Specifies non-coherent dram address offset
 *
 * @retval - read data
 * 
 * 
 *****************************************************************************/

#define DRAM_READ(addr) READ_LWORD(((addr)) + Hal_dram_ch0_virtAddr)

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Writes a longword data to DRAM location.
 *      
 * @param chan - IN Specifies dram channel. 0 is non-cohernet dram, 1 is 
 *                  coherent dram
 * @param addr - IN Specifies dram address offset
 * @param val - IN Specifies value to write
 *
 * @retval - none
 * 
 * 
 *****************************************************************************/

#define DRAM_WRITE_XA(chan, addr, val) {\
    if(chan==1) {WRITE_LWORD_CACHED((addr) + Hal_dram_ch1_virtAddr, (val));} \
    else {WRITE_LWORD((addr) + Hal_dram_ch0_virtAddr, (val));}}

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Writes a longword data to non-coherent DRAM location.
 *      
 * @param addr - IN Specifies non-coherent dram address offset
 * @param val - IN Specifies value to write
 *
 * @retval - none
 * 
 * 
 *****************************************************************************/

#define DRAM_WRITE_CH0(addr, val) WRITE_LWORD(((addr)) + Hal_dram_ch0_virtAddr, (val))

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Writes a longword data to coherent DRAM location.
 *      
 * @param addr - IN Specifies coherent dram address offset
 * @param val - IN Specifies value to write
 *
 * @retval - none
 * 
 * 
 *****************************************************************************/

#define DRAM_WRITE_CH1(addr, val) WRITE_LWORD_CACHED(((addr)) + Hal_dram_ch1_virtAddr, (val))

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Reads a longword data from non-coherent DRAM location.
 *      
 * @param addr - IN Specifies non-coherent dram address offset
 *
 * @retval - read data
 * 
 * 
 *****************************************************************************/

#define DRAM_READ_CH0(addr) READ_LWORD((addr) + Hal_dram_ch0_virtAddr)

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Reads a longword data from coherent DRAM location.
 *      
 * @param addr - IN Specifies coherent dram address offset
 *
 * @retval - read data
 * 
 * 
 *****************************************************************************/

#define DRAM_READ_CH1(addr) READ_LWORD_CACHED((addr) + Hal_dram_ch1_virtAddr)

#endif  /* __HAL_DRAM_H */ 

