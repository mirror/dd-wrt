/**
 **************************************************************************
 * @file uof_prf.h
 *
 * @description
 *      This is the header file for uCode Object File
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

/*
 ****************************************************************************
 * Doxygen group definitions
 ****************************************************************************/

/**
 *****************************************************************************
 * @file uof_prf.h
 * 
 * @defgroup uof_prf UOF Microcode File Format Definition for Profiler Chunk
 * 
 * @ingroup UOF
 *
 * @description
 *      This header file that contains the profile chunk definitions 
 *      of micorcode file format used by linker and loader.
 *
 *****************************************************************************/
 
#ifndef __UOF_PRF_H__
#define __UOF_PRF_H__

#include "uof.h"

#define PRF_OBJS	"PRF_OBJS"      /**< profile section ID */
#define PRF_IMAG	"PRF_IMAG"      /**< profile image section ID */
#define PRF_STRT	"PRF_STRT"      /**< profile string table section ID */

typedef struct prf_ObjTable_S{
   unsigned int		numEntries;		/**< num of entries in table */
   /* numEntries * sizeof(element) follows */
}prf_ObjTable_T;


typedef uof_strTab_T	prf_strTab_T;
typedef uof_chunkHdr_T	prf_chunkHdr_T;



typedef struct prf_Image_S{
   int			lstFileName;		/**< list file name str-table offset */
   unsigned int aeAssigned;			/**< bit values of assigned MEs */
   unsigned int	prfTabSize;			/**< source lines table size */
   unsigned int	prfTabOffset;		/**< source lines table offset */
   unsigned int	uAddrVirtOffset;	/**< virtual uaddr relocation offset */
}prf_Image_T;



typedef struct prf_Info_S{
   int			prfInfo;			/**< source lines str-table offset */
   unsigned int reserved1;			/**< reserved for future use */
}prf_Info_T;

#endif			// __UOF_PRF_H__
