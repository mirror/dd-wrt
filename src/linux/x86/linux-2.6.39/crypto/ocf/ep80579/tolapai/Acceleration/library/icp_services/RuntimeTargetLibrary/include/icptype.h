/**
 **************************************************************************
 * @file icptype.h
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

#ifndef __ICPTYPE_H
#define __ICPTYPE_H

/* The following are used to determine the product type in the .list file.
   They are used in various Tools to implement chip-specific code. 
   This is NOT the actual product type.  The values must be consistent
   across all the Tools.
*/

#define EP80579_CPU_TYPE                (0x00020000)

/* Please don't delete any values from the list below unless you know what you
   are doing.  Also, add new values to the end of the list and prior to the
   ICP_ANY_REG value. These values are used in the UOF and arbitrarily
   changing them may have an undesiriable effect.
*/
typedef enum{
    ICP_NO_DEST,           /**< no destination */
    ICP_GPA_REL,           /**< general-purpose A register under relative mode */
    ICP_GPA_ABS,           /**< general-purpose A register under absolute mode */
    ICP_GPB_REL,           /**< general-purpose B register under relative mode */
    ICP_GPB_ABS,           /**< general-purpose B register under absolute mode */
    ICP_SR_REL,            /**< sram register under relative mode */
    ICP_SR_RD_REL,         /**< sram read register under relative mode */
    ICP_SR_WR_REL,         /**< sram write register under relative mode */
    ICP_SR_ABS,            /**< sram register under absolute mode */
    ICP_SR_RD_ABS,         /**< sram read register under absolute mode */
    ICP_SR_WR_ABS,         /**< sram write register under absolute mode */
    ICP_SR0_SPILL,         /**< sram0 spill register */
    ICP_SR1_SPILL,         /**< sram1 spill register */
    ICP_SR2_SPILL,         /**< sram2 spill register */
    ICP_SR3_SPILL,         /**< sram3 spill register */
    ICP_SR0_MEM_ADDR,      /**< sram0 memory address register */
    ICP_SR1_MEM_ADDR,      /**< sram1 memory address register */
    ICP_SR2_MEM_ADDR,      /**< sram2 memory address register */
    ICP_SR3_MEM_ADDR,      /**< sram3 memory address register */
    ICP_DR_REL,            /**< dram register under relative mode */
    ICP_DR_RD_REL,         /**< dram read register under relative mode */
    ICP_DR_WR_REL,         /**< dram write register under relative mode */
    ICP_DR_ABS,            /**< dram register under absolute mode */
    ICP_DR_RD_ABS,         /**< dram read register under absolute mode */
    ICP_DR_WR_ABS,         /**< dram write register under absolute mode */
    ICP_DR_MEM_ADDR,       /**< dram memory address register */
    ICP_LMEM,              /**< local memory */
    ICP_LMEM0,             /**< local memory bank0 */
    ICP_LMEM1,             /**< local memory bank1 */
    ICP_LMEM_SPILL,        /**< local memory spill */
    ICP_LMEM_ADDR,         /**< local memory address */
    ICP_NEIGH_REL,         /**< next neighbour register under relative mode */
    ICP_NEIGH_INDX,        /**< next neighbour register under index mode */
    ICP_SIG_REL,           /**< signal register under relative mode */ 
    ICP_SIG_INDX,          /**< signal register under index mode */ 
    ICP_SIG_DOUBLE,        /**< signal register */ 
    ICP_SIG_SINGLE,        /**< signal register */ 
    ICP_SCRATCH_MEM_ADDR,  /**< scratch memory address */ 
    ICP_UMEM0,             /**< ustore memory bank0 */ 
    ICP_UMEM1,             /**< ustore memory bank1 */ 
    ICP_UMEM_SPILL,        /**< ustore memory spill */ 
    ICP_UMEM_ADDR,         /**< ustore memory address */ 
    ICP_DR1_MEM_ADDR,      /**< dram channel1 address */  
    ICP_SR0_IMPORTED,      /**< sram channel0 imported data */  
    ICP_SR1_IMPORTED,      /**< sram channel1 imported data */  
    ICP_SR2_IMPORTED,      /**< sram channel2 imported data */  
    ICP_SR3_IMPORTED,      /**< sram channel3 imported data */   
    ICP_DR_IMPORTED,       /**< dram channel0 imported data */  
    ICP_DR1_IMPORTED,      /**< dram channel1 imported data */  
    ICP_SCRATCH_IMPORTED,  /**< scratch imported data */  
    ICP_XFER_RD_ABS,       /**< transfer read register under absolute mode */  
    ICP_XFER_WR_ABS,       /**< transfer write register under absolute mode */  
    ICP_CONST_VALUE,       /**< const alue */  
    ICP_ADDR_TAKEN,        /**< address taken */  
    ICP_OPTIMIZED_AWAY,    /**< optimized away */  
    ICP_SHRAM_ADDR,        /**< shared ram address */  
    ICP_ANY_REG = 0xffff   /**< any register */   
}icp_RegType_T;

#define ICP_SR_INDX     ICP_SR_ABS    /**< sram transfer register under index mode */  
#define ICP_DR_INDX     ICP_DR_ABS    /**< dram transfer register under index mode */  
#define ICP_NEIGH_ABS   ICP_NEIGH_INDX /**< next neighbor register under absolute mode */  

#ifdef WIN32
typedef unsigned __int64    uint64;
typedef __int64             int64;
#else
typedef unsigned long long  uint64;
typedef long long           int64;
#endif

typedef uint64 uword_T;

#endif  /* ifdef __ICPTYPE_H */
