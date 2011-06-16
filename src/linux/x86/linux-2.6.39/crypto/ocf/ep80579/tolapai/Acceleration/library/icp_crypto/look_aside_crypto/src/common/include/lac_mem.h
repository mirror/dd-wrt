/***************************************************************************
 *
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
 ***************************************************************************/

/**
 ***************************************************************************
 * @file lac_mem.h
 *
 * @defgroup LacMem     Memory
 *
 * @ingroup LacCommon
 *
 * Memory re-alignment functions and memory accessor macros.
 *
 ***************************************************************************/


#ifndef LAC_MEM_H
#define LAC_MEM_H

/***************************************************************************
 * Include public/global header files
 ***************************************************************************/
#include "cpa.h"
#include "IxOsal.h"
#include "lac_common.h"

/**
 *******************************************************************************
 * @ingroup LacMem
 *      These macros are used to Endian swap variables from IA to QAT.
 *
 * @param[out] x    The variable to be swapped.
 *
 * @retval none
 ******************************************************************************/
#if (LAC_BYTE_ORDER==__LITTLE_ENDIAN)
#  define LAC_MEM_WR_64(x) IX_OSAL_HOST_TO_NW_64(x)
#  define LAC_MEM_WR_32(x) IX_OSAL_HOST_TO_NW_32(x)
#  define LAC_MEM_WR_16(x) IX_OSAL_HOST_TO_NW_16(x)
#  define LAC_MEM_RD_64(x) IX_OSAL_NW_TO_HOST_64(x)
#  define LAC_MEM_RD_32(x) IX_OSAL_NW_TO_HOST_32(x)
#  define LAC_MEM_RD_16(x) IX_OSAL_NW_TO_HOST_16(x)
#else
#  define LAC_MEM_WR_64(x) (x)
#  define LAC_MEM_WR_32(x) (x)
#  define LAC_MEM_WR_16(x) (x)
#  define LAC_MEM_RD_64(x) (x)
#  define LAC_MEM_RD_32(x) (x)
#  define LAC_MEM_RD_16(x) (x)
#endif

/*
*******************************************************************************
* Shared Memory Macros (memory accessible by Acceleration Engines, e.g. QAT)
*******************************************************************************
*/

/**
 *******************************************************************************
 * @ingroup LacMem
 *      This macro can be used to write to a variable that will be read by the
 * QAT. The macro operates on 8-bit values.
 *
 * @param[out] var    The variable to be written. Can be a field of a struct.
 *
 * @param[in] data    The value to be written.  Will be cast to the size of the
 *                    target.
 *
 * @retval none
 ******************************************************************************/
#define LAC_MEM_SHARED_WRITE_8BIT(var, data)        \
do {                                                \
     (var) = (Cpa8U)(data);                         \
} while(0)

/**
 *******************************************************************************
 * @ingroup LacMem
 *      This macro can be used to write to a variable that will be read by the
 * QAT. The macro operates on 16-bit values.
 *
 * @param[out] var    The variable to be written. Can be a field of a struct.
 *
 * @param[in] data    The value to be written.  Will be cast to the size of the
 *                    target.
 *
 * @retval none
 ******************************************************************************/
#define LAC_MEM_SHARED_WRITE_16BIT(var, data)       \
do {                                                \
       (var) = (Cpa16U)(data);                      \
       (var) = LAC_MEM_WR_16(((Cpa16U) var));       \
} while(0)

/**
 *******************************************************************************
 * @ingroup LacMem
 *      This macro can be used to write to a variable that will be read by the
 * QAT. The macro operates on 32-bit values.
 *
 * @param[out] var    The variable to be written. Can be a field of a struct.
 *
 * @param[in] data    The value to be written.  Will be cast to the size of the
 *                    target.
 *
 * @retval none
 ******************************************************************************/
#define LAC_MEM_SHARED_WRITE_32BIT(var, data)       \
do {                                                \
       (var) = (Cpa32U)(data);                      \
       (var) = LAC_MEM_WR_32(((Cpa32U) var));       \
} while(0)


/**
 *******************************************************************************
 * @ingroup LacMem
 *      This macro can be used to write to a variable that will be read by the
 * QAT. The macro operates on 64-bit values.
 *
 * @param[out] var    The variable to be written. Can be a field of a struct.
 *
 * @param[in] data    The value to be written.  Will be cast to the size of the
 *                    target.
 *
 * @retval none
 ******************************************************************************/
#define LAC_MEM_SHARED_WRITE_64BIT(var, data)       \
do {                                                \
       (var) = (Cpa64U)(data);                      \
       (var) = LAC_MEM_WR_64(((Cpa64U) var));       \
} while(0)


/**
 *******************************************************************************
 * @ingroup LacMem
 *      This macro can be used to read a variable that was written by the QAT.
 * The macro will automatically detect the size of the data to be read and will
 * select the correct method for performing the read. The value read from the
 * variable is cast to the size of the data type it will be stored in.
 *
 * @param[in] var     The variable to be read. Can be a field of a struct.
 *
 * @param[out] data   The variable to hold the result of the read. Data read
 *                    will be cast to the size of this variable
 *
 * @retval none
 ******************************************************************************/
#define LAC_MEM_SHARED_READ(var, data)                \
do {                                                  \
    switch(sizeof(var))                               \
    {                                                 \
        case 1:                                       \
            (data) = (var);                           \
            break;                                    \
        case 2:                                       \
            (data) = LAC_MEM_RD_16(((Cpa16U) var));   \
            break;                                    \
        case 4:                                       \
            (data) = LAC_MEM_RD_32(((Cpa32U) var));   \
            break;                                    \
        case 8:                                       \
            (data) = LAC_MEM_RD_64(((Cpa64U) var));   \
            break;                                    \
        default:                                      \
            break;                                    \
    }                                                 \
} while(0)


/**
 *******************************************************************************
 * @ingroup LacMem
 *      This macro can be used to write a pointer to a QAT request. The fields
 *      for pointers in the QAT request and response messages are always 64 bits
 *
 * @param[out] var    The variable to be written to. Can be a field of a struct.
 *
 * @param[in] data    The value to be written.  Will be cast to size of target
 *                    variable
 *
 * @retval none
 ******************************************************************************/
/* cast pointer to scalar of same size of the native pointer */
#define LAC_MEM_SHARED_WRITE_FROM_PTR(var, data) \
    ((var) = (Cpa64U)(unsigned long)(data))

/* Note: any changes to this macro implementation should also be made to the
 * similar LAC_MEM_CAST_PTR_TO_UINT64 macro
 */

/**
 *******************************************************************************
 * @ingroup LacMem
 *      This macro can be used to read a pointer from a QAT response. The fields
 *      for pointers in the QAT request and response messages are always 64 bits
 *
 * @param[in] var     The variable to be read. Can be a field of a struct.
 *
 * @param[out] data   The variable to hold the result of the read. Data read
 *                    will be cast to the size of this variable
 *
 * @retval none
 ******************************************************************************/
/* Cast back to native pointer */
#define LAC_MEM_SHARED_READ_TO_PTR(var, data)   \
    ((data) = (void *)(unsigned long)(var))


/**
 *******************************************************************************
 * @ingroup LacMem
 *      This macro safely casts a pointer to a Cpa64U type.
 *
 * @param[in] pPtr   The pointer to be cast.
 *
 * @retval pointer cast to Cpa64U
 ******************************************************************************/
#define LAC_MEM_CAST_PTR_TO_UINT64(pPtr) \
    ((Cpa64U)(unsigned long)(pPtr))

/**
 *******************************************************************************
 * @ingroup LacMem
 *      This macro uses an OSAL macro to convert from a virtual address to a
 *      physical address.
 *
 * @param[in] pVirtAddr   The address to be converted.
 *
 * @retval The converted physical address
 ******************************************************************************/
#define LAC_OS_VIRT_TO_PHYS(pVirtAddr) \
    (IX_OSAL_OS_MMU_VIRT_TO_PHYS(pVirtAddr))



/**
 *******************************************************************************
 * @ingroup LacMem
 *      This macro can be used to write an address variable that will be read by
 * the QAT.  The macro will perform the necessary virt2phys address translation
 *
 * @param[out] var  The address variable to write. Can be a field of a struct.
 *
 * @param[in] pPtr  The pointer variable to containing the address to be
 *                  written
 *
 * @retval none
 ******************************************************************************/
#define LAC_MEM_SHARED_WRITE_VIRT_TO_PHYS_PTR(var, pPtr)                    \
do {                                                                        \
    Cpa64U physAddr = 0;                                                    \
    physAddr = LAC_MEM_CAST_PTR_TO_UINT64(LAC_OS_VIRT_TO_PHYS(pPtr));       \
    LAC_MEM_SHARED_WRITE_64BIT((var), physAddr);                                  \
} while (0)


/*
*******************************************************************************
* OS Memory Macros
*******************************************************************************
*/

/**
 *******************************************************************************
 * @ingroup LacMem
 *      This function and associated macro allocates the memory for the given 
 *      size and stores the address of the memory allocated in the pointer.
 *
 * @param[out] ppMemAddr    address of pointer where address will be stored
 * @param[in] sizeBytes     the size of the memory to be allocated.
 *
 * @retval CPA_STATUS_RESOURCE  Macro failed to allocate Memory
 * @retval CPA_STATUS_SUCCESS   Macro executed successfully
 *
 ******************************************************************************/
static __inline CpaStatus
LacMem_OsMemAlloc(void **ppMemAddr,
                  Cpa32U sizeBytes)
{
    *ppMemAddr = ixOsalMemAlloc(sizeBytes);
    if (NULL == *ppMemAddr)
    {
        return CPA_STATUS_RESOURCE;
    }

    return CPA_STATUS_SUCCESS;
}

/**
 *******************************************************************************
 * @ingroup LacMem
 *      This function and associated macro allocates the contiguous 
 *       memory for the given 
 *      size and stores the address of the memory allocated in the pointer.
 *
 * @param[out] ppMemAddr     address of pointer where address will be stored
 * @param[in] sizeBytes      the size of the memory to be allocated.
 * @param[in] alignmentBytes the alignment
 *
 * @retval CPA_STATUS_RESOURCE       Macro failed to allocate Memory
 * @retval CPA_STATUS_SUCCESS        Macro executed successfully
 * @retval CPA_STATUS_INVALID_PARAM  Macro failed in parameters verification
 *
 ******************************************************************************/
static __inline CpaStatus
LacMem_OsContigAlignMemAlloc(void **ppMemAddr,
                  Cpa32U sizeBytes, Cpa32U alignmentBytes)
{
#if defined(ICP_PARAM_CHECK)
    if((alignmentBytes & (alignmentBytes - 1)) != 0) /* if is not power of 2 */
    {
        *ppMemAddr = NULL;
        LAC_INVALID_PARAM_LOG("alignmentBytes MUST be the power of 2;\r\n");
        return CPA_STATUS_INVALID_PARAM;
    }
#endif
    *ppMemAddr = ixOsalMemAllocAligned(0, /*padding unused*/
                                       sizeBytes, 
                                       alignmentBytes);
    if (NULL == *ppMemAddr)
    {
        return CPA_STATUS_RESOURCE;
    }

    return CPA_STATUS_SUCCESS;
}

/**
 *******************************************************************************
 * @ingroup LacMem
 *      Macro from the LacMem_OsMemAlloc function
 *
 ******************************************************************************/

#define LAC_OS_MALLOC(ppMemAddr, sizeBytes) \
    LacMem_OsMemAlloc((void **) (ppMemAddr), (sizeBytes))

/**
 *******************************************************************************
 * @ingroup LacMem
 *      Macro from the LacMem_OsContigAlignMemAlloc function
 *
 ******************************************************************************/
#define LAC_OS_CAMALLOC(ppMemAddr, sizeBytes, alignmentBytes) \
    LacMem_OsContigAlignMemAlloc((void **) ppMemAddr, sizeBytes, alignmentBytes)

/**
 *******************************************************************************
 * @ingroup LacMem
 *      Macro for declaration static const unsigned int constant. One provides
 *   the compilation time computation with the highest bit set in the 
 *   sizeof(TYPE) value. The constant is being put by the linker by default in 
 *   .rodata section 
 * 
 *   E.g. Statement LAC_DECLARE_HIGHEST_BIT_OF(lac_mem_blk_t)
 *   results in following entry: 
 *     static const unsigned int highest_bit_of_lac_mem_blk_t = 3
 *
 *   CAUTION!!
 *      Macro is prepared only for type names NOT-containing ANY
 *  special characters. Types as amongst others: 
 *  - void *
 *  - unsigned long
 *  - unsigned int
 *  are strictly forbidden and will result in compilation error.
 *  Use typedef to provide one-word type name for MACRO's usage.
 ******************************************************************************/
#define LAC_DECLARE_HIGHEST_BIT_OF(TYPE)            \
  static const unsigned int highest_bit_of_##TYPE = \
    (sizeof(TYPE) & 0x80000000 ? 31 :               \
    (sizeof(TYPE) & 0x40000000 ? 30 :               \
    (sizeof(TYPE) & 0x20000000 ? 29 :               \
    (sizeof(TYPE) & 0x10000000 ? 28 :               \
    (sizeof(TYPE) & 0x08000000 ? 27 :               \
    (sizeof(TYPE) & 0x04000000 ? 26 :               \
    (sizeof(TYPE) & 0x02000000 ? 25 :               \
    (sizeof(TYPE) & 0x01000000 ? 24 :               \
    (sizeof(TYPE) & 0x00800000 ? 23 :               \
    (sizeof(TYPE) & 0x00400000 ? 22 :               \
    (sizeof(TYPE) & 0x00200000 ? 21 :               \
    (sizeof(TYPE) & 0x00100000 ? 20 :               \
    (sizeof(TYPE) & 0x00080000 ? 19 :               \
    (sizeof(TYPE) & 0x00040000 ? 18 :               \
    (sizeof(TYPE) & 0x00020000 ? 17 :               \
    (sizeof(TYPE) & 0x00010000 ? 16 :               \
    (sizeof(TYPE) & 0x00008000 ? 15 :               \
    (sizeof(TYPE) & 0x00004000 ? 14 :               \
    (sizeof(TYPE) & 0x00002000 ? 13 :               \
    (sizeof(TYPE) & 0x00001000 ? 12 :               \
    (sizeof(TYPE) & 0x00000800 ? 11 :               \
    (sizeof(TYPE) & 0x00000400 ? 10 :               \
    (sizeof(TYPE) & 0x00000200 ?  9 :               \
    (sizeof(TYPE) & 0x00000100 ?  8 :               \
    (sizeof(TYPE) & 0x00000080 ?  7 :               \
    (sizeof(TYPE) & 0x00000040 ?  6 :               \
    (sizeof(TYPE) & 0x00000020 ?  5 :               \
    (sizeof(TYPE) & 0x00000010 ?  4 :               \
    (sizeof(TYPE) & 0x00000008 ?  3 :               \
    (sizeof(TYPE) & 0x00000004 ?  2 :               \
    (sizeof(TYPE) & 0x00000002 ?  1 :               \
    (sizeof(TYPE) & 0x00000001 ?  0 : 32) ))))))))))))))))/*16*/))))))))))))))) /* 31 */

/**
 *******************************************************************************
 * @ingroup LacMem
 *      This function and associated macro allocates the memory for the given 
 *      size and stores the address of the memory allocated in the pointer. This
 *      atomic malloc is interrupt-safe.
 *
 * @param[out] ppMemAddr    address of pointer where address will be stored
 * @param[in] sizeBytes     the size of the memory to be allocated.
 *
 * @retval CPA_STATUS_RESOURCE  Macro failed to allocate Memory
 * @retval CPA_STATUS_SUCCESS   Macro executed successfully
 *
 ******************************************************************************/
static __inline CpaStatus
LacMem_OsMemAllocAtomic(void **ppMemAddr,
                        Cpa32U sizeBytes)
{
    *ppMemAddr = ixOsalMemAllocAtomic(sizeBytes);
    if ((*ppMemAddr = ixOsalMemAllocAtomic(sizeBytes)) == NULL)
    {
        return CPA_STATUS_RESOURCE;
    }

    return CPA_STATUS_SUCCESS;
}

#define LAC_OS_MALLOC_ATOMIC(ppMemAddr, sizeBytes) \
    LacMem_OsMemAllocAtomic((void **) (ppMemAddr), (sizeBytes))


/**
 *******************************************************************************
 * @ingroup LacMem
 *      This function and associated macro frees the memory at the given address
 *      and resets the pointer to NULL
 *
 * @param[out] ppMemAddr    address of pointer where mem address is stored.
 *                          If pointer is NULL, the function will exit silently
 *
 * @retval void
 *
 ******************************************************************************/
static __inline void
LacMem_OsMemFree(void **ppMemAddr)
{
    if (NULL != *ppMemAddr)
    {
        ixOsalMemFree(*ppMemAddr);
        *ppMemAddr = NULL;
    }
}

/**
 *******************************************************************************
 * @ingroup LacMem
 *      This function and associated macro frees the contiguous memory at the 
 *      given address and resets the pointer to NULL
 *
 * @param[out] ppMemAddr    address of pointer where mem address is stored.
 *                          If pointer is NULL, the function will exit silently
 * @param[in]  sizeBytes    size of memory we are about to free - unused 
 *                          on Linux
 *
 * @retval void
 *
 ******************************************************************************/
static __inline void
LacMem_OsContigAlignMemFree(void **ppMemAddr, Cpa32U sizeBytes)
{
    if (NULL != *ppMemAddr)
    {
        ixOsalMemAlignedFree(*ppMemAddr, sizeBytes);
        *ppMemAddr = NULL;
    }
}

#define LAC_OS_FREE(pMemAddr) \
    LacMem_OsMemFree((void **) &pMemAddr)

#define LAC_OS_CAFREE(pMemAddr, sizeBytes) \
    LacMem_OsContigAlignMemFree((void **) &pMemAddr, sizeBytes)

/**
 *******************************************************************************
 * @ingroup LacMem
 *      enum to choose between interrupt safe allocation or not 
 * 
 ******************************************************************************/
typedef enum 
{
    LAC_MEM_ALLOC_TYPE_NORMAL = 0,
    /**< Normal allocation */
    LAC_MEM_ALLOC_TYPE_ATOMIC
    /**< Interrupt safe allocation */
}lac_mem_alloc_type_t; 


/**
 *******************************************************************************
 * @ingroup LacMem
 * This function and associated macros allocate a specified amount of memory 
 * where a physical byte alignment for the memory block can be specified. The 
 * address returned is a virtual address.  
 *
 * @param[out] ppMemAddr    the address of the pointer to which the allocated
 *                          memory will be assigned.
 * @param[in] sizeBytes     the amount of memory (in bytes) to allocate
 * @param[in] alignment     alignment required (in bytes)
 * @param[in] allocType     Specify normal or atomic allocation
 *
 * @retval CPA_STATUS_SUCCESS       Function executed successfully.
 * @retval CPA_STATUS_RESOURCE      Function failed to allocate memory.
 *
 ******************************************************************************/
static __inline CpaStatus
LacMem_OsMemAllocAligned(void **ppMemAddr, Cpa32U sizeBytes, 
                         Cpa32U alignment, lac_mem_alloc_type_t allocType)
{
    Cpa8U *pMemOrginal  = NULL;
    Cpa8U *pAlignedVirt = NULL;
    Cpa32U sizeToAlloc = sizeBytes + alignment + sizeof(void *);   
 
    /* allocate extra space to ensure
     * 1. enough space to advance the pointer forward for the memory allocated 
     *    to the next aligned address.
     * 2. enough space to store the original pointer to the memory allocated.
     *    This is stored just before the aligned address.
     *
     *      ++++++++++++++++++++++++++++++    
     *      | pad | orig addr |          |
     *      ++++++++++++++++++++++++++++++
     *      ^                 ^   
     *      |                 |
     *  orig addr      aligned addr
     */
    if (LAC_MEM_ALLOC_TYPE_ATOMIC == allocType)
    {
        pMemOrginal = (Cpa8U *)ixOsalMemAllocAtomic(sizeToAlloc); 
    }
    else
    {
        pMemOrginal = (Cpa8U *)ixOsalMemAlloc(sizeToAlloc);
    }

    if (NULL == pMemOrginal)
    {
        return CPA_STATUS_RESOURCE;
    }

    if (0 != alignment)
    {
        /* Add on size of the pointer to the original address and then get
         * the physical address so we can get the next physically aligned
         * address */
        unsigned long physAddress = (unsigned long) 
            LAC_OS_VIRT_TO_PHYS(pMemOrginal + sizeof(void *));
        unsigned long physAddressAlign = 
            (unsigned long) LAC_ALIGN_POW2_ROUNDUP(physAddress, alignment);

        pAlignedVirt = (Cpa8U *) (pMemOrginal + sizeof(void *) +
                       (physAddressAlign - physAddress));
            
    }
    else
    {
        pAlignedVirt = (Cpa8U *) (pMemOrginal + sizeof(void *));
    }
    
    /* write the original pointer just before the aligned address */
    *((unsigned long *)pAlignedVirt - 1) = (unsigned long) pMemOrginal; 

    *ppMemAddr = pAlignedVirt;

    return CPA_STATUS_SUCCESS;
}

#define LAC_OS_MALLOC_ALIGNED(ppMemAddr, sizeBytes, alignment) \
    LacMem_OsMemAllocAligned((void *)ppMemAddr, (sizeBytes), \
                             (alignment), LAC_MEM_ALLOC_TYPE_NORMAL) 

#define LAC_OS_MALLOC_ATOMIC_ALIGNED(ppMemAddr, sizeBytes, alignment) \
    LacMem_OsMemAllocAligned((void *)ppMemAddr, (sizeBytes), \
                             (alignment), L8AC_MEM_ALLOC_TYPE_ATOMIC)


/**
 *******************************************************************************
 * @ingroup LacMem
 * This function frees memory which has been allocated with the aligned 
 * allocation function.
 *
 * @param[out] ppAlignedMem     the address of the pointer from which
 *                              the aligned memory will be freed.
 * @retval void
 *
 ******************************************************************************/
static __inline void
LacMem_OsMemFreeAligned(void **ppAlignedMem)
{
    void *pMemOrginal = NULL;

    if (NULL == *ppAlignedMem)
    {
        return;
    }

    /* retrieve the original address which is stored in the memory 
     * just before the aligned address */
    pMemOrginal = (void *) *((unsigned long *)*ppAlignedMem - 1);

    ixOsalMemFree(pMemOrginal);
    *ppAlignedMem = NULL;
}

#define LAC_OS_FREE_ALIGNED(pMemAddr) \
    LacMem_OsMemFreeAligned((void *)&pMemAddr)

/*
*******************************************************************************
* Memory Re-alignment Functions
*******************************************************************************
*/

/**
 * @ingroup LacMem
 * @brief Type definition to control copying between two different size
 *        memory areas.
 * @description
 *        Type definition to control copying between two different size
 *        memory areas. Right padding is similar to memcpy; bytes are copied
 *        from the beginning of the source area to the beginning of the
 *        destination area. If the destination is smaller than the source, the
 *        data is truncated. If the destination is larger than the source, there
 *        will be padding remaining in the right side of the destination.
 *
 *        Left padding is used for copying large numbers. Copying is done from
 *        the end of the source buffer to the end of the destination buffer,
 *        working back to the start of the buffers. If the source buffer is
 *        larger than the destination buffer, it is truncated. If the
 *        destination is larger than the source, there will be padding remaining
 *        in the left side (beginning) of the destination.
 *
 *        Bitwise OR your padding choice with ICP_LAC_MEM_PAD_ZEROES to force
 *        the alignment/restore functions to overwrite any padded areas with
 *        zeroes.
 *
 * Examples:
 *
 * ICP_LAC_MEM_PAD_RIGHT
 *
 *      source              [abcdefghijk]
 *      larger destination  [abcdefghijk????]
 *      larger destination  [abcdefghijk0000] if ICP_LAC_MEM_PAD_ZEROES is used
 *      smaller destination [abcdef]
 *
 * ICP_LAC_MEM_PAD_LEFT
 *
 *      source              [abcdefghijk]
 *      larger destination  [????abcdefghijk]
 *      larger destination  [0000abcdefghijk] if ICP_LAC_MEM_PAD_ZEROES is used
 *      smaller destination [cdefghijk]
 */
typedef enum
{
    ICP_LAC_MEM_DEFAULT    = LAC_BIT(0),
    ICP_LAC_MEM_NO_COPY    = LAC_BIT(0),
    ICP_LAC_MEM_PAD_RIGHT  = LAC_BIT(1),
    ICP_LAC_MEM_PAD_LEFT   = LAC_BIT(2),
    ICP_LAC_MEM_PAD_ZEROES = LAC_BIT(15)
} icp_lac_mem_padding_t;



/***************************************************************************
 * Prototypes
 ***************************************************************************/


/**
*******************************************************************************
 * @ingroup LacMem
 *     Aligns a buffer into a working buffer
 *
 * @description
 *      This function produces an 8-byte aligned working buffer from the input
 *      user buffer. If the original buffer is not aligned, or it is too small,
 *      a new buffer shall be allocated and memory is copied unless the user
 *      specifies ICP_LAC_MEM_NO_COPY or ICP_LAC_MEM_DEFAULT as padding control.
 *      The output (working) buffer shall be copied otherwise, but padding
 *      is significant only if the working length is greater than the user
 *      length. Note that padding always describes how the output (working)
 *      buffer shall be padded.
 *
 *      The returned working buffer is guaranteed to be 8-byte aligned and of
 *      the desired size. If the working length is greater than the user length
 *      a reallocation shall always happen even if the original buffer was
 *      aligned.
 *
 *      The caller must keep the original buffer pointer. The aligned buffer is
 *      freed (as necessary) using LacMem_BufferRestore().
 *
 * @param[in] user_buffer the user buffer (may or may not be 8 byte aligned)
 * @param[in] user_len    length of the user buffer
 * @param[in] working_len length of the working (aligned) buffer
 * @param[in] padding     copy and padding type control
 *
 * @return a pointer to the working (aligned) buffer or NULL if the allocation
 *      failed
 *
 * @note the working length cannot be smaller than the user buffer length
 *
 * @warning the working buffer may be the same or different from the original
 * user buffer; the caller should make no assumptions in this regard
 *
 * @see LacMem_BufferRestore()
 *
 ******************************************************************************/
Cpa8U *
LacMem_BufferAlign(Cpa8U *user_buffer,
                   Cpa32U user_len,
                   Cpa32U working_len,
                   icp_lac_mem_padding_t padding);


/**
*******************************************************************************
 * @ingroup LacMem
 *     Restores a user buffer
 *
 * @description
 *      This function restores a user buffer and releases its
 *      corresponding working buffer. The working buffer, assumed to be
 *      previously obtained using LacMem_BufferAlign(), is freed as necessary.

 *      The contents are copied in the process as specified by the padding
 *      parameter, which describe the padding of the source (working) buffer.
 *      This function will not pad the destination (user) buffer in any way.
 *      Soliciting zero padding will give an error when used with this function.
 *
 * @note the working length cannot be smaller than the user buffer length
 *
 * @param[out] user_buffer      the user buffer
 * @param[in] user_len          length of the user buffer
 * @param[in] working_buffer    working (aligned) buffer
 * @param[in] working_len       working (aligned) buffer length
 * @param[in] padding           copy and padding type control
 *
 * @return the status of the operation
 *
 * @see LacMem_BufferAlign()
 *
 ******************************************************************************/
CpaStatus
LacMem_BufferRestore(Cpa8U *user_buffer,
                     Cpa32U user_len,
                     Cpa8U *working_buffer,
                     Cpa32U working_len,
                     icp_lac_mem_padding_t padding);

/*
*******************************************************************************
 * @ingroup LacMem
 *     Allocates the buffers used for aligning
 *
 * @description
 *      This function allocates the buffers to be used in the align and
 *      restore functions. A mem pool is created to hold the buffers. This
 *      function must be called before any aligning takes place.
 * 
 * @param[in] numAsymConcurrentReq  number of asymmetric requests. 
 *
 * @return the status of the operation
 *
 ******************************************************************************/
CpaStatus
LacMem_InitBuffers(Cpa64U numAsymConcurrentReq);

/*
*******************************************************************************
 * @ingroup LacMem
 *     Destroys the buffers used for aligning
 *
 * @description
 *      This function destroys the buffers to be used in the align and
 * restore functions. The mem pool is destroyed along with the buffers. This
 * function must not be called if any aligned memory is in use.
 *
 ******************************************************************************/
void
LacMem_DestroyBuffers(void);

#endif /* LAC_MEM_H */
