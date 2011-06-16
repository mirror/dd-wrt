/**
 * @file IxOsalBufferMgt.h
 *
 * @brief OSAL Buffer pool management and buffer management definitions.
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
/* @par
 * -- Copyright Notice --
 *
 * @par
 * Copyright 1979, 1980, 1983, 1986, 1988, 1989, 1991, 1992, 1993, 1994 
 *      The Regents of the University of California. All rights reserved.
 *
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * @par
 * -- End of Copyright Notice --
 */

#ifndef IxOsalBufferMgt_H
#define IxOsalBufferMgt_H
#ifdef __cplusplus
extern "C"{
#endif
#include "IxOsal.h"
/**
 * @defgroup IxOsalBufferMgt OSAL Buffer Management Module.
 *
 * @brief Buffer management module for IxOsal
 *
 * @{ 
 */

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_MAX_POOLS
 *
 * @brief The maximum number of pools that can be allocated, must be 
 *        a multiple of 32 as required by implementation logic.
 * @note  This can safely be increased if more pools are required.
 */
#define IX_OSAL_MBUF_MAX_POOLS      32 

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_NAME_LEN
 *
 * @brief The maximum string length of the pool name
 */
#define IX_OSAL_MBUF_POOL_NAME_LEN  64



/**
 *  Define IX_OSAL_MBUF
 */


/* forward declaration of internal structure */
struct __ACP_BUF;

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_ATTRIBUTE_ALIGN32 
 *
 * @brief OS can define it in IxOsalOs.h to skip the following
 * definition.
 */
#ifndef IX_OSAL_ATTRIBUTE_ALIGN32
#define IX_OSAL_ATTRIBUTE_ALIGN32 __attribute__ ((aligned(32)))
#endif

/* release v1.4 backward compatible definitions */
struct __IX_MBUF
{
    struct __ACP_BUF *ix_next IX_OSAL_ATTRIBUTE_ALIGN32;
                                /**< MBUF next pointer */
    struct __ACP_BUF *ix_nextPacket;
                                /**< Pointer to next packet */
    UINT8 *ix_data;             /**< Data pointer */
    UINT32 ix_len;              /**< Buffer length */
    UINT8  ix_type;             /**< Type of buffer */
    UINT8  ix_flags;            /**< Flags */
    UINT16 ix_reserved;         /**< Reserved area */
    UINT32 ix_rsvd;             /**< Reserve area-2 */
    UINT32 ix_PktLen;           /**< Packet length */
    VOID *ix_priv;              /**< Private pointer */
};

/* Control header structure */ 
struct __IX_CTRL
{
    UINT32 ix_reserved[2];        /**< Reserved field */
    UINT32 ix_signature;          /**< Field to indicate if buffers are allocated by the system */    
    UINT32 ix_allocated_len;      /**< Allocated buffer length */  
    UINT32 ix_allocated_data;     /**< Allocated buffer data pointer */  
    VOID *ix_pool;                /**< pointer to the buffer pool */
    struct __ACP_BUF *ix_chain;   /**< chaining */ 
    VOID *ix_osbuf_ptr;           /**< Storage for OS-specific buffer pointer */
};

#ifdef _DIAB_TOOL

IX_OSAL_ATTRIBUTE_ALIGN32 struct __IX_NE_SHARED
{
            UINT32 reserved[8];   /**< Reserved area for PIU Service-specific usage */
};

#else

/* NE-Shared structure */
struct __IX_NE_SHARED
{
            UINT32 reserved[8] IX_OSAL_ATTRIBUTE_ALIGN32;   /**< Reserved area for PIU  Service-specific usage */
};

#endif


/* 
 * ACP buffer structure 
 */
struct __ACP_BUF
{
    struct __IX_MBUF ix_mbuf IX_OSAL_ATTRIBUTE_ALIGN32; /**< buffer header */
    struct __IX_CTRL ix_ctrl;                           /**< buffer management */
    struct __IX_NE_SHARED ix_ne;                        /**< Reserved area for PIU Service-specific usage*/
};



/**
 * @ingroup IxOsalBufferMgt
 *
 * @brief IX_OSAL_MBUF
 *
 * @note Generic ACP mbuf format.
 */
typedef struct __ACP_BUF IX_OSAL_MBUF;


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(m_blk_ptr)
 *
 * @brief Return pointer to the next mbuf in a single packet
 */
#define IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(m_blk_ptr)  \
        (m_blk_ptr)->ix_mbuf.ix_next


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(m_blk_ptr)
 *
 * @brief Return pointer to the next packet in the chain
 */
#define IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(m_blk_ptr)  \
        (m_blk_ptr)->ix_mbuf.ix_nextPacket


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_MDATA(m_blk_ptr)
 *
 * @brief Return pointer to the data in the mbuf
 */
#define IX_OSAL_MBUF_MDATA(m_blk_ptr)       (m_blk_ptr)->ix_mbuf.ix_data

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_MLEN(m_blk_ptr)
 *
 * @brief Return the data length
 */
#define IX_OSAL_MBUF_MLEN(m_blk_ptr) \
    (m_blk_ptr)->ix_mbuf.ix_len

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_MTYPE(m_blk_ptr)
 *
 * @brief Return the data type in the mbuf
 */
#define IX_OSAL_MBUF_MTYPE(m_blk_ptr) \
    (m_blk_ptr)->ix_mbuf.ix_type


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_FLAGS(m_blk_ptr)
 *
 * @brief Return the buffer flags
 */
#define IX_OSAL_MBUF_FLAGS(m_blk_ptr)       \
        (m_blk_ptr)->ix_mbuf.ix_flags


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_NET_POOL(m_blk_ptr)
 *
 * @brief Return pointer to a network pool
 */
#define IX_OSAL_MBUF_NET_POOL(m_blk_ptr)	\
        (m_blk_ptr)->ix_ctrl.ix_pool



/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_PKT_LEN(m_blk_ptr)
 *
 * @brief Return the total length of all the data in
 * the mbuf chain for this packet
 */
#define IX_OSAL_MBUF_PKT_LEN(m_blk_ptr) \
        (m_blk_ptr)->ix_mbuf.ix_PktLen




/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_PRIV(m_blk_ptr)
 *
 * @brief Return the private field
 */
#define IX_OSAL_MBUF_PRIV(m_blk_ptr)        \
        (m_blk_ptr)->ix_mbuf.ix_priv



/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_SIGNATURE(m_blk_ptr)
 *
 * @brief Return the signature field of IX_OSAL_MBUF
 */
#define IX_OSAL_MBUF_SIGNATURE(m_blk_ptr)  \
        (m_blk_ptr)->ix_ctrl.ix_signature


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_OSBUF_PTR(m_blk_ptr)
 *
 * @brief Return ix_osbuf_ptr field of IX_OSAL_MBUF, which is used to store OS-specific buffer pointer during a buffer conversion.
 */
#define IX_OSAL_MBUF_OSBUF_PTR(m_blk_ptr)  \
        (m_blk_ptr)->ix_ctrl.ix_osbuf_ptr


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_ALLOCATED_BUFF_LEN(m_blk_ptr)
 *
 * @brief Return the allocated buffer size
 */
#define IX_OSAL_MBUF_ALLOCATED_BUFF_LEN(m_blk_ptr)  \
        (m_blk_ptr)->ix_ctrl.ix_allocated_len

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_ALLOCATED_BUFF_DATA(m_blk_ptr)
 *
 * @brief Return the allocated buffer pointer
 */
#define IX_OSAL_MBUF_ALLOCATED_BUFF_DATA(m_blk_ptr)  \
        (m_blk_ptr)->ix_ctrl.ix_allocated_data



/* Name length */
#define IX_OSAL_MBUF_POOL_NAME_LEN  64


/****************************************************
 * Macros for buffer pool management
 ****************************************************/

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_FREE_COUNT
 *
 * @brief Return the total number of freed buffers left in the pool.
 */
#define IX_OSAL_MBUF_POOL_FREE_COUNT(m_pool_ptr) \
                    ixOsalBuffPoolFreeCountGet(m_pool_ptr)

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_SIZE_ALIGN
 *
 * @brief This macro takes an integer as an argument and
 * rounds it up to be a multiple of the memory cache-line 
 * size.
 *
 * @param size  IN - the size integer to be rounded up
 *
 * @return int - the size, rounded up to a multiple of
 *               the cache-line size
 */
#if !defined(__linux_user) && !defined(__freebsd_user)

#define IX_OSAL_MBUF_POOL_SIZE_ALIGN(size)                 \
    ((((size) + (IX_OSAL_CACHE_LINE_SIZE - 1)) /      \
        IX_OSAL_CACHE_LINE_SIZE) *                  \
            IX_OSAL_CACHE_LINE_SIZE)

#else /* __linux_user || __freebsd_user */

#define IX_OSAL_MBUF_POOL_SIZE_ALIGN(size)	(size)
	
#endif /* __linux_user || __freebsd_user  */

/**
 * @ingroup IxOsalBufferMgtInternal
 *
 * @brief This calculates, from the number of mbufs required, the 
 * size of the memory area required to contain the mbuf headers for the
 * buffers in the pool.  The size to be used for each mbuf header is 
 * rounded up to a multiple of the cache-line size, to ensure
 * each mbuf header aligns on a cache-line boundary.
 *
 * @param count - the number of buffers the pool will contain
 *
 * @return int - the total size required for the pool mbuf area (aligned)
 *
 * @note Don't use this directly, use macro
 */
PUBLIC UINT32 ixOsalBuffPoolMbufAreaSizeGet (UINT32 count);


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_MBUF_AREA_SIZE_ALIGNED
 *
 * @brief This macro calculates, from the number of mbufs required, the 
 * size of the memory area required to contain the mbuf headers for the
 * buffers in the pool.  The size to be used for each mbuf header is 
 * rounded up to a multiple of the cache-line size, to ensure
 * each mbuf header aligns on a cache-line boundary.
 * This macro is used by IX_OSAL_MBUF_POOL_MBUF_AREA_ALLOC()
 *
 * @param count   IN - the number of buffers the pool will contain
 *
 * @return int - the total size required for the pool mbuf area (aligned)
 */
#define IX_OSAL_MBUF_POOL_MBUF_AREA_SIZE_ALIGNED(count) \
        ixOsalBuffPoolMbufAreaSizeGet(count)


/**
 * @ingroup IxOsalBufferMgtInternal
 *
 * @brief This macro calculates, from the number of mbufs required and the
 * size of the data portion for each mbuf, the size of the data memory area
 * required. The size is adjusted to ensure alignment on cache line boundaries.
 * This macro is used by IX_OSAL_MBUF_POOL_DATA_AREA_ALLOC()
 *
 *
 * @param count      IN  - The number of mbufs in the pool.
 * @param size       IN  - The desired size for each mbuf data portion.
 *                         This size will be rounded up to a multiple of the
 *                         cache-line size to ensure alignment on cache-line
 *                         boundaries for each data block.
 *
 * @return the total size required for the pool data area (aligned)
 *
 * @note Don't use this directly, use macro
 */
PUBLIC UINT32 ixOsalBuffPoolDataAreaSizeGet (UINT32 count, UINT32 size);


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_DATA_AREA_SIZE_ALIGNED
 *
 * @brief This macro calculates, from the number of mbufs required and the
 * size of the data portion for each mbuf, the size of the data memory area
 * required. The size is adjusted to ensure alignment on cache line boundaries.
 * This macro is used by IX_OSAL_MBUF_POOL_DATA_AREA_ALLOC()
 *
 *
 * @param count    IN    - The number of mbufs in the pool.
 * @param size     IN    - The desired size for each mbuf data portion.
 *                         This size will be rounded up to a multiple of the
 *                         cache-line size to ensure alignment on cache-line
 *                         boundaries for each data block.
 *
 * @return int - the total size required for the pool data area (aligned)
 */
#define IX_OSAL_MBUF_POOL_DATA_AREA_SIZE_ALIGNED(count, size) \
        ixOsalBuffPoolDataAreaSizeGet((count), (size))


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_MBUF_AREA_ALLOC
 *
 * @brief Allocates the memory area needed for the number of mbuf headers
 * specified by <i>count</i>.
 * This macro ensures the mbuf headers align on cache line boundaries.
 * This macro evaluates to a pointer to the memory allocated.
 *
 * @param count        IN - the number of mbufs the pool will contain
 * @param memAreaSize  OUT - the total amount of memory allocated
 *
 * @return void * - a pointer to the allocated memory area
 */
#if !defined(__linux_user) && !defined(__freebsd_user)

#define IX_OSAL_MBUF_POOL_MBUF_AREA_ALLOC(count, memAreaSize) \
    IX_OSAL_CACHE_DMA_MALLOC((memAreaSize =                 \
        IX_OSAL_MBUF_POOL_MBUF_AREA_SIZE_ALIGNED(count)))

#else /* __linux_user || __freebsd_user */

#define IX_OSAL_MBUF_POOL_MBUF_AREA_ALLOC(count, memAreaSize) \
	IX_OSAL_BUFF_MEM_ALLOC(count * memAreaSize)

#endif /* __linux_user || __freebsd_user */

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_DATA_AREA_ALLOC
 *
 * @brief Allocates the memory pool for the data portion of the pool mbufs.
 * The number of mbufs is specified by <i>count</i>.  The size of the data
 * portion of each mbuf is specified by <i>size</i>.
 * This macro ensures the mbufs are aligned on cache line boundaries
 * This macro evaluates to a pointer to the memory allocated.
 *
 * @param count    IN   - the number of mbufs the pool will contain
 * @param size     IN   - the desired size (in bytes) required for the data
 *                        portion of each mbuf.  Note that this size may be
 *                        rounded up to ensure alignment on cache-line
 *                        boundaries.
 * @param memAreaSize IN - the total amount of memory allocated
 *
 * @return void * - a pointer to the allocated memory area
 */
#if !defined(__linux_user) && !defined(__freebsd_user)

#define IX_OSAL_MBUF_POOL_DATA_AREA_ALLOC(count, size, memAreaSize) \
    IX_OSAL_CACHE_DMA_MALLOC((memAreaSize =                     \
        IX_OSAL_MBUF_POOL_DATA_AREA_SIZE_ALIGNED(count,size)))

#else /* __linux_user */

#define IX_OSAL_MBUF_POOL_DATA_AREA_ALLOC(count, size, memAreaSize) \
	IX_OSAL_BUFF_MEM_ALLOC(count * size * memAreaSize)

#endif /* __linux_user || freebsd_user */


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_INIT
 *
 * @brief Wrapper macro for ixOsalPoolInit() 
 * See function description below for details.
 */
#define IX_OSAL_MBUF_POOL_INIT(count, size, name) \
    ixOsalPoolInit((count), (size), (name))

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_NO_ALLOC_POOL_INIT
 *
 * @return Pointer to the new pool or NULL if the initialization failed.
 *
 * @brief Wrapper macro for ixOsalNoAllocPoolInit() 
 * See function description below for details.
 * 
 */
#define IX_OSAL_MBUF_NO_ALLOC_POOL_INIT(bufPtr, dataPtr, count, size, name) \
    ixOsalNoAllocPoolInit( (bufPtr), (dataPtr), (count), (size), (name))

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_GET
 *
 * @brief Wrapper macro for ixOsalMbufAlloc() 
 * See function description below for details.
 */
#define IX_OSAL_MBUF_POOL_GET(poolPtr) \
        ixOsalMbufAlloc(poolPtr)

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_PUT
 *
 * @brief Wrapper macro for ixOsalMbufFree() 
 * See function description below for details.
 */
#define IX_OSAL_MBUF_POOL_PUT(bufPtr) \
    ixOsalMbufFree(bufPtr)

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_PUT_CHAIN
 *
 * @brief Wrapper macro for ixOsalMbufChainFree() 
 * See function description below for details.
 */
#define IX_OSAL_MBUF_POOL_PUT_CHAIN(bufPtr) \
    ixOsalMbufChainFree(bufPtr)

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_SHOW
 *
 * @brief Wrapper macro for ixOsalMbufPoolShow() 
 * See function description below for details.
 */
#define IX_OSAL_MBUF_POOL_SHOW(poolPtr) \
    ixOsalMbufPoolShow(poolPtr)

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_PIU_PRIV_SECTION_PTR 
 *
 * @brief Wrapper macro for getting pointer to PIU(Programmable I/O Unit)
 *         private section
 * 
 */
#define IX_OSAL_PIU_PRIV_SECTION_PTR(buffer) buffer->ix_ne.reserved 

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_MDATA_RESET
 *
 * @brief Wrapper macro for ixOsalMbufDataPtrReset() 
 * See function description below for details.
 */
#define IX_OSAL_MBUF_POOL_MDATA_RESET(bufPtr) \
    ixOsalMbufDataPtrReset(bufPtr)

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_UNINIT
 *
 * @brief Wrapper macro for ixOsalBuffPoolUninit() 
 * See function description below for details.
 */
#define IX_OSAL_MBUF_POOL_UNINIT(m_pool_ptr)  \
        ixOsalBuffPoolUninit(m_pool_ptr)

/*
 * Added in Phase 2
 */

/* BUFFER INFO FLUSHING */
/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_BUFF_FLUSH_INFO 
 *
 * @brief Flush buffer info 
 */
#if !defined(__linux_user) && !defined(__freebsd_user) 

#define IX_OSAL_BUFF_FLUSH_INFO IX_OSAL_CACHE_FLUSH

#else

#define IX_OSAL_BUFF_FLUSH_INFO(ptr,size) 

#endif /* __linux_user || __freebsd_user */

/* MEMORY ALLOCATION IN BUFFER MANAGEMENT MODULE */
/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_BUFF_MEM_ALLOC
 *
 * @brief Buffer memory allocation 
 */
#if !defined(__linux_user) && !defined(__freebsd_user)

#define IX_OSAL_BUFF_MEM_ALLOC IX_OSAL_CACHE_DMA_MALLOC

#else

#define IX_OSAL_BUFF_MEM_ALLOC ixOsalMemAlloc

#endif /* __linux_user || __freebsd_user */

/* MEMORY FREE IN BUFFER MANAGEMENT MODULE */
/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_BUFF_MEM_FREE 
 *
 * @brief Buffer memory free
 */
#if !defined(__linux_user) && !defined(__freebsd_user)

#define IX_OSAL_BUFF_MEM_FREE IX_OSAL_CACHE_DMA_FREE

#else /* __linux_user || __freebsd_user */

#define IX_OSAL_BUFF_MEM_FREE ixOsalMemFree

#endif /* __linux_user || __freebsd_user */


/* 
 * Include OS-specific bufferMgt definitions 
 */
#include "IxOsalOsBufferMgt.h"


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_CONVERT_OSBUF_TO_ACPBUF
 *
 * @brief Convert pre-allocated os-specific buffer format to OSAL ACP_BUF (IX_OSAL_MBUF) format. 
 * It is users' responsibility to provide pre-allocated and valid buffer pointers.
 * @param osBufPtr (in) - a pre-allocated os-specific buffer pointer.
 * @param acpBufPtr (in)- a pre-allocated OSAL ACP_BUF pointer
 * @return None
 */
#define IX_OSAL_CONVERT_OSBUF_TO_ACPBUF(osBufPtr,acpBufPtr) \
        IX_OSAL_OS_CONVERT_OSBUF_TO_ACPBUF(osBufPtr,acpBufPtr)        


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_CONVERT_ACPBUF_TO_OSBUF
 *
 * @brief Convert pre-allocated OSAL ACP_BUF (IX_OSAL_MBUF) format to os-specific buffer pointers.
 * @param acpBufPtr (in) - OSAL ACP_BUF pointer
 * @param osBufPtr (out) - os-specific buffer pointer.
 * @return None
 */

#define IX_OSAL_CONVERT_ACPBUF_TO_OSBUF(acpBufPtr,osBufPtr)  \
        IX_OSAL_OS_CONVERT_ACPBUF_TO_OSBUF(acpBufPtr,osBufPtr)


/**
 * @ingroup IxOsalBufferMgt
 *
 * @brief Initialize MBUF pool
 *
 * @param count    IN - number of buffers to be in pool
 * @param size     IN - size of each buffer
 * @param name     IN - name for the pool  
 *
 * Initialize a pool of mbufs with given count / size 
 *
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - handle to the newly created/initialized pool
 */
PUBLIC IX_OSAL_MBUF_POOL *ixOsalPoolInit (UINT32 count,
                      UINT32 size, const char *name);

/**
 * @ingroup IxOsalBufferMgt
 *
 * @brief Initialize pool without allocating data space
 *
 * @param poolBufPtr    - Buffer head pointer
 * @param poolDataPtr   - Data pointer for buffer pool
 * @param count         - number of buffers expected to be in pool
 * @param size          - size of pool
 * @param name          - name for buffer pool 
 *
 * Initialize buffer pool without actually allocating data space
 *
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - Handle to newly initialized pool
 */
PUBLIC IX_OSAL_MBUF_POOL *ixOsalNoAllocPoolInit (VOID *poolBufPtr,
                                                 VOID *poolDataPtr,
						 UINT32 count,
						 UINT32 size,
						 const char *name);

/**
 * @ingroup IxOsalBufferMgt
 *
 * @brief Allocate buffer
 *
 * @param pool - pool handle
 *
 * Allcoate a buffer from the pool
 *
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - handle to allocated buffer
 */
PUBLIC IX_OSAL_MBUF *ixOsalMbufAlloc (IX_OSAL_MBUF_POOL * pool);

/**
 * @ingroup IxOsalBufferMgt
 *
 * @brief Free buffer
 *
 * @param mbuf - buffer handle
 *
 * Free buffer pointer to by handle
 *
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - Buffer handle
 */
PUBLIC IX_OSAL_MBUF *ixOsalMbufFree (IX_OSAL_MBUF * mbuf);

/**
 * @ingroup IxOsalBufferMgt
 *
 * @brief Free buffer chain 
 *
 * @param mbuf - buffer handle
 *
 * Free the chain of buffers pointer to by given handle
 *
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - None
 */
PUBLIC VOID ixOsalMbufChainFree (IX_OSAL_MBUF * mbuf);

/**
 * @ingroup IxOsalBufferMgt
 *
 * @brief Reset data pointer for buffer
 *
 * @param mbuf - buffer pointer
 *
 * Reset the data pointer field in buffer
 *
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - None
 */
PUBLIC VOID ixOsalMbufDataPtrReset (IX_OSAL_MBUF * mbuf);

/**
 * @ingroup IxOsalBufferMgt
 *
 * @brief Show buffer pool info
 *
 * @param pool  - pool handle
 *
 * Show info for buffer pool given by handle 
 *
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - None
 */
PUBLIC VOID ixOsalMbufPoolShow (IX_OSAL_MBUF_POOL * pool);

/**
 * @ingroup IxOsalBufferMgt
 *
 * @brief Uninitialize buffer pool
 *
 * @param pool  - pool handle
 *
 * Free up memory and destroy/uninitialize buffer pool
 *
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS upon success, IX_FAIL otherwise
 */
PUBLIC IX_STATUS ixOsalBuffPoolUninit (IX_OSAL_MBUF_POOL * pool);

/**
 * @ingroup IxOsalBufferMgt
 *
 * @brief Get count of free buffers in pool 
 *
 * @param pool  -  pool handle
 *
 * Returns the number of free buffers in given pool
 *
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - 32-bit count of number of free buffers
 */
PUBLIC UINT32 ixOsalBuffPoolFreeCountGet(IX_OSAL_MBUF_POOL * pool);


/**
 * @} IxOsalBufferMgt
 */
#ifdef __cplusplus
}
#endif

#endif /* IxOsalBufferMgt_H */
