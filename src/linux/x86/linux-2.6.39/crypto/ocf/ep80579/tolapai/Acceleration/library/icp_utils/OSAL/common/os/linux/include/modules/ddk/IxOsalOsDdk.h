/**
 * @file IxOsalOsDdk.h
 *
 * @brief Linux-specific OS Ddk definitions
 *
 * Design Notes:
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
 */

#ifndef IxOsalOsDdk_H
#define IxOsalOsDdk_H

/*
 * Below macros and defines are used for OSAL CacheMMU APIs .
 */

/*
 * Definition of what is deemed a small memory allocation request.
 * Memory requests for up to this size are deemed small and are
 * handled differently from larger memory requests
 */
#define IX_OSAL_OS_SMALL_MEM_SIZE        (512 - 32)

/* Arbitrary numbers to detect memory corruption */
#define IX_OSAL_OS_MAGIC_ALLOC_NUMBER    (0xBABEFACE)
#define IX_OSAL_OS_MAGIC_DEALLOC_NUMBER  (0xCAFEBABE)

/* Number of information words maintained behind the user buffer */
#define IX_OSAL_OS_NUM_INFO_WORDS        (4)

/* Number of bytes per word */
#define IX_OSAL_OS_BYTES_PER_WORD        (4)

/* Index of information words maintained behind user buffer */
#define IX_OSAL_OS_ORDER_OF_PAGES_INDEX  (-4)
#define IX_OSAL_OS_MYPTR_INDEX           (-3)
#define IX_OSAL_OS_REQUESTED_SIZE_INDEX  (-2)
#define IX_OSAL_OS_MAGIC_NUMBER_INDEX    (-1)

/* Macro to round up a size to a multiple of a cache line */
#define IX_OSAL_OS_CL_ROUND_UP(s) \
      (((s) + (IX_OSAL_CACHE_LINE_SIZE - 1)) & ~(IX_OSAL_CACHE_LINE_SIZE - 1))


#endif /* IxOsalOsDdk_H */
