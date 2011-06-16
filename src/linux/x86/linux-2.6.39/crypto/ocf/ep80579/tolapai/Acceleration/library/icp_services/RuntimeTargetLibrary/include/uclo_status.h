/**
 **************************************************************************
 * @file uclo_status.h
 *
 * @description
 *      This is the header file for uCode Loader Library Status Codes
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

#ifndef __UCLOSTATUS_H__
#define __UCLOSTATUS_H__

enum{
    UCLO_SUCCESS        = 0,         /**< the operation was successful */
    UCLO_FAILURE        = 0x8200,    /**< the operation failed */
    UCLO_FILEFAIL       = 0x8201,    /**< file operation failed */
    UCLO_BADOBJ         = 0x8202,    /**< bad object */
    UCLO_MEMFAIL        = 0x8203,    /**< memory access fail */
    UCLO_BADARG         = 0x8204,    /**< bad function argument */
    UCLO_NOOBJ          = 0x8205,    /**< no object */
    UCLO_IMGNOTFND      = 0x8206,    /**< image not found */
    UCLO_SYMNOTFND      = 0x8207,    /**< symbol not found */
    UCLO_NEIGHNOTFND    = 0x8208,    /**< neighbour AE not found */
    UCLO_UOFINCOMPAT    = 0x820b,    /**< UOF is incompatible with the chip type/revision */
    UCLO_UOFVERINCOMPAT = 0x820c,    /**< UOF version incompatible with the loader -- mismatched uof format */
    UCLO_UNINITVAR      = 0x820e,    /**< uninitialized import variable */
    UCLO_EXPRFAIL       = 0x820f,    /**< expression fail */
    UCLO_EXCDDRAMSIZE   = 0x8210,    /**< address excede none-coherent dram size */
    UCLO_EXCDDRAM1SIZE  = 0x8211,    /**< address excede coherent dram size */
    UCLO_EXCDSRAM0SIZE  = 0x8212,    /**< address excede sram size */
    UCLO_EXCDSCRTHSIZE  = 0x8213,    /**< address excede scratch size */
    UCLO_EXCDLMEMSIZE   = 0x8214,    /**< address excede local memory size */
    UCLO_INVLDCTX       = 0x8215,    /**< invalid context */
    UCLO_EXCDUMEMSIZE   = 0x8216,    /**< address excede ustore memory size */
    UCLO_ADDRNOTFOUND   = 0x8217,    /**< address not found */
    UCLO_PAGENOTFOUND   = 0x8218,    /**< page not found */
    UCLO_IVDWARN        = 0x8219,    /**< unknown image or symbol defined */
    UCLO_EXCDSHRAMSIZE  = 0x821a     /**< address excede shared ram size */
};                      

#endif  /* __UCLOSTATUS_H__ */
