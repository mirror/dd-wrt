/**
 * @file IxOsalBufferMgtDefault.h
 *
 * @brief Default buffer pool management and buffer management
 *        definitions.
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

#ifndef IX_OSAL_BUFFER_MGT_DEFAULT_H
#define IX_OSAL_BUFFER_MGT_DEFAULT_H
#ifdef __cplusplus
extern "C"{
#endif

/**
 * @ingroup IxOsalBufferMgt
 *
 * @enum IxOsalMbufPoolAllocationType
 *
 * @brief Used to indicate how the pool memory was allocated
 */
typedef enum
{
    IX_OSAL_MBUF_POOL_TYPE_SYS_ALLOC = 0, /**< mbuf pool allocated by the system */
    IX_OSAL_MBUF_POOL_TYPE_USER_ALLOC	 /**< mbuf pool allocated by the user */
} IxOsalMbufPoolAllocationType;


/**
 * @ingroup IxOsalBufferMgt
 *
 * @brief IxOsalMbufPool 
 *
 * @note Implementation of buffer pool structure for use with non-VxWorks OS
 */
typedef struct
{
    IX_OSAL_MBUF *nextFreeBuf;	/**< Pointer to the next free mbuf              */
    VOID *mbufMemPtr;	   /**< Pointer to the mbuf memory area            */
    VOID *dataMemPtr;	   /**< Pointer to the data memory area            */
    INT32 bufDataSize;	   /**< The size of the data portion of each mbuf  */
    INT32 totalBufsInPool;   /**< Total number of mbufs in the pool          */
    INT32 freeBufsInPool;	   /**< Number of free mbufs currently in the pool */
    INT32 mbufMemSize;	   /**< The size of the pool mbuf memory area      */
    INT32 dataMemSize;	   /**< The size of the pool data memory area      */
    char name[IX_OSAL_MBUF_POOL_NAME_LEN + 1];	 /**< Descriptive name for pool */
    IxOsalMbufPoolAllocationType poolAllocType; /**< Pool Allocation type */
    UINT32 poolIdx;  /**< Pool Index */ 
} IxOsalMbufPool;

/**
 * @ingroup IxOsalBufferMgt
 *
 * @brief IX_OSAL_MBUF_POOL 
 *
 * @note Implementation of buffer pool structure for use with non-VxWorks OS
 */
typedef IxOsalMbufPool IX_OSAL_MBUF_POOL;


/* 
 *  PUBLIC IX_STATUS ixOsalBuffPoolUninit (IX_OSAL_MBUF_POOL * pool);
 */
#ifdef __cplusplus
}
#endif
#endif /* IX_OSAL_BUFFER_MGT_DEFAULT_H */
