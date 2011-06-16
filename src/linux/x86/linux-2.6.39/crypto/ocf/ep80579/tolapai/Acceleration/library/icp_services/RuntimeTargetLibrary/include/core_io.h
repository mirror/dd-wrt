/**
 **************************************************************************
 * @file core_io.h
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
 * @file core_io.h
 * 
 * @ingroup icp_hal
 *
 * @description
 *      This is Hardware Abstraction Layer header file and it lists all basic
 *      memory and register access macros and functions.
 *
 *****************************************************************************/
   
#ifndef CORE_IO_H
#define CORE_IO_H

#include "icptype.h"
#include "core_platform.h"
 
/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Swap bits of a 16-bit data
 *      
 * @param x - IN 16-bit data to swap
 *
 * @retval - none
 * 
 * 
 *****************************************************************************/

 #define ENDIAN_SWAP16(x) \
     ((((unsigned short)(x) & 0x00ff) << 8) | \
    (((unsigned short)(x) & 0xff00) >> 8))

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Swap bits of a 32-bit data
 *      
 * @param x - IN 32-bit data to swap
 *
 * @retval - none
 * 
 * 
 *****************************************************************************/

 #define ENDIAN_SWAP32(x) \
    ((((unsigned int)(x) & 0x000000ff) << 24) | \
     (((unsigned int)(x) & 0x0000ff00) <<  8) | \
     (((unsigned int)(x) & 0x00ff0000) >>  8) | \
     (((unsigned int)(x) & 0xff000000) >> 24))

/**
 *****************************************************************************
 * @ingroup icp_hal
 * 
 * @description
 *      Swap bits of a 64-bit data
 *      
 * @param x - IN 64-bit data to swap
 *
 * @retval - none
 * 
 * 
 *****************************************************************************/

 #define ENDIAN_SWAP64(x) \
    ((((uint64)(x) & ((uint64)0xff)) << 56) | \
     (((uint64)(x) & ((uint64)0xff << 8)) << 40) | \
     (((uint64)(x) & ((uint64)0xff << 16)) << 24) | \
     (((uint64)(x) & ((uint64)0xff << 24)) << 8) | \
     (((uint64)(x) & ((uint64)0xff << 32)) >> 8) | \
     (((uint64)(x) & ((uint64)0xff << 40)) >> 24) | \
     (((uint64)(x) & ((uint64)0xff << 48)) >> 40) | \
     (((uint64)(x) & ((uint64)0xff << 56)) >> 56))

#define VA_BIT_UNMAPPED  36
#define VA_MASK_ALL (((uint64)(-1)) << VA_BIT_UNMAPPED)

#endif /* CORE_IO_H */
