/**
 * @file IxOsalDdk.h
 *
 * @brief include file for OSAL's DDK module. 
 * 
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

#ifndef IxOsalDdk_H
#define IxOsalDdk_H
#ifdef __cplusplus
extern "C"{
#endif
/* Basic types */
#include "IxOsal.h"

/* 
 * Include OS-specific ddk definitions 
 */
#include "IxOsalOsDdk.h"
#if (!defined(__freebsd))
/** 
 * @ingroup IxOsal
 *
 * @brief physical to virtual address translation
 *
 * @param physAddr - physical address
 *
 * Converts a physical address into its equivalent MMU-mapped virtual address
 *
 * @li Reentrant: no
 * @li IRQ safe:  yes
 *
 * @return Corresponding virtual address, as UINT32
 */
#define IX_OSAL_MMU_PHYS_TO_VIRT(physAddr) \
    IX_OSAL_OS_MMU_PHYS_TO_VIRT(physAddr)
        
#endif /*!defined(__freebsd)*/

/** 
 * @ingroup IxOsal
 *
 * @brief virtual to physical address translation
 *
 * @param virtAddr - virtual address
 *
 * Converts a virtual address into its equivalent MMU-mapped physical address
 *
 * @li Reentrant: no
 * @li IRQ safe:  yes
 *
 * @return Corresponding physical address, as UINT32
 */
#define IX_OSAL_MMU_VIRT_TO_PHYS(virtAddr)  \
    IX_OSAL_OS_MMU_VIRT_TO_PHYS(virtAddr)



/** 
 * @ingroup IxOsal
 *
 * @brief cache to memory flush
 *
 * @param addr - memory address to flush from cache
 * @param size - number of bytes to flush (rounded up to a cache line)
 *
 * Flushes the cached value of the memory zone pointed by "addr" into memory,
 * rounding up to a cache line. Use before the zone is to be read by a
 * processing unit which is not cache coherent with the main CPU.
 *
 * @li Reentrant: no
 * @li IRQ safe:  yes
 *
 * @return - none
 */
#define IX_OSAL_CACHE_FLUSH(addr, size)  IX_OSAL_OS_CACHE_FLUSH(addr, size)



/** 
 * @ingroup IxOsal
 *
 * @brief cache line invalidate
 *
 * @param addr - memory address to invalidate in cache
 * @param size - number of bytes to invalidate (rounded up to a cache line)
 *
 * Invalidates the cached value of the memory zone pointed by "addr",
 * rounding up to a cache line. Use before reading the zone from the main
 * CPU, if the zone has been updated by a processing unit which is not cache
 * coherent with the main CPU.
 *
 * @li Reentrant: no
 * @li IRQ safe:  yes
 *
 * @return - none
 */
#define IX_OSAL_CACHE_INVALIDATE(addr, size)  IX_OSAL_OS_CACHE_INVALIDATE(addr, size)

/**
 * @ingroup IxOsal
 *
 * @brief cache line preload
 *
 * @param addr - memory address to cache
 * @param size - number of bytes to cache (rounded up to a cache line)
 *
 *
 * Preloads a section of memory to the cache memory in multiples of cache line size.
 *
 * @li Reentrant: no
 * @li IRQ safe:  yes
 *
 * @return - none
 */
#define IX_OSAL_CACHE_PRELOAD(addr, size)  IX_OSAL_OS_CACHE_PRELOAD(addr, size)
#ifdef __cplusplus
}
#endif 
#endif
