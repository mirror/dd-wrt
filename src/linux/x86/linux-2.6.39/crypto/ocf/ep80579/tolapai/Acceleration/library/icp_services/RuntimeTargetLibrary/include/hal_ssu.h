/**
 **************************************************************************
 * @file hal_ssu.h
 *
 * @description
 *      This is the header file for Hardware Abstraction Layer -- SSU Unit
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
 * @file hal_ssu.h
 * 
 * @ingroup icp_hal
 *
 * @description
 *      This header file contains the macros of SSU access
 *
 *****************************************************************************/
 
#ifndef __HAL_SSU_H
#define __HAL_SSU_H

#include "core_io.h"
#include "halMmap.h"

typedef enum{
    /* Use these in the GET/PUT_SSU_CSR macros */
    SSU_INTMASKSSU        = 0x0000,
    SSU_INTSTATSSU        = 0x0004,
    SSU_PPERR             = 0x0008,
    SSU_PPERRID           = 0x000C,
    SSU_CERRSSUSH         = 0x0010,
    SSU_CERSSUSHAD        = 0x0014,
    SSU_UERRSSUSH         = 0x0018,
    SSU_UERRSSUSHAD       = 0x001C,
    SSU_CBOVRDSSUSH       = 0x0020,
    SSU_CLKCFGPSSSUSH     = 0x0024,
    SSU_CLKCFGSPSSUSH     = 0x0028,
    /* Use these in the GET/PUT_SSU_MMP_CSR macros - pass MMP number [0-1] in the chan parameter */
    SSU_CERRSSUMMP        = 0x0380,
    SSU_CERRSSUMMPAD      = 0x0384,
    SSU_UERRSSUMMP        = 0x0388,
    SSU_UERRSSUMMPAD      = 0x038C,
    SSU_CBOVRDSSUMMP      = 0x0390,
    SSU_DBPCSRSSUMMP      = 0x03C0,
    SSU_DBPPCSSUMMP       = 0x03C4,
    SSU_DIOVRDSSUMMP      = 0x03C8,
    SSU_DMWISSUMMP        = 0x03CC,
    SSU_DOAWDSSUMMP       = 0x03D0,
    SSU_DOBWDSSUMMP       = 0x03D4,
    SSU_DPMWDSSUMMP       = 0x03D8,
    SSU_DMRISSUMMP        = 0x03DC,
    SSU_DOARDSSUMMP       = 0x03E0,
    SSU_DOBRDSSUMMP       = 0x03E4,
    SSU_DPMRDSSUMMP       = 0x03E8,
    SSU_DSMRDSSUMMP       = 0x03EC,
    SSU_DISRDSSUMMP       = 0x03F0
}Hal_Ssu_CSR_T;

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Writes a longword data to SSU control and status register.
 *      
 * @param csr - IN Specifies register offset
 * @param val - IN Specifies value to write
 *
 * @retval - none
 * 
 * 
 *****************************************************************************/

#define PUT_SSU_CSR(csr, val)  WRITE_LWORD((Hal_ssu_csr_virtAddr + ((csr) & 0x3FF)), (val))

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Reads a longword data from SSU control and status register.
 *      
 * @param csr - IN Specifies register offset
 *
 * @retval - read data
 * 
 * 
 *****************************************************************************/

#define GET_SSU_CSR(csr)       READ_LWORD(Hal_ssu_csr_virtAddr + ((csr) & 0x3FF))

#define SSU_MMP_CSR(chan) (Hal_ssu_csr_virtAddr + 0x400*(chan & 0x1))

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Writes a longword data to SSU MMP control and status register.
 *      
 * @param chan - IN Specifies MMM number
 * @param csr - IN Specifies register offset
 * @param val - IN Specifies value to write
 *
 * @retval - none
 * 
 * 
 *****************************************************************************/
 
#define PUT_SSU_MMP_CSR(chan, csr, val)  \
              WRITE_LWORD((SSU_MMP_CSR(chan)+((csr) & 0x3FF)), (val))

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Reads a longword data from SSU MMP control and status register.
 *      
 * @param chan - IN Specifies MMM number
 * @param csr - IN Specifies register offset
 *
 * @retval - data read
 * 
 * 
 *****************************************************************************/

#define GET_SSU_MMP_CSR(chan, csr) \
              READ_LWORD((SSU_MMP_CSR(chan)+((csr) & 0x3FF)))

#endif      /* __HAL_SSU_H */
