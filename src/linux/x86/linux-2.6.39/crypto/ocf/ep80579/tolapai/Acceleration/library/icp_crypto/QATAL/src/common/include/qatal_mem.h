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
 * @file qatal_mem.h
 *
 * @defgroup QatalMem	  Memory Functions and Macros
 *
 * @ingroup icp_Qatal
 *
 * @description
 *	This files defines common memory re-alignment functions and
 *	memory accessor macros.
 *
 ***************************************************************************/


/***************************************************************************/

#ifndef QATAL_MEM_H
#define QATAL_MEM_H

/***************************************************************************
 * Include public/global header files
 ***************************************************************************/
#include "cpa.h"
#include "qatal_common.h"
#include "IxOsal.h"
#include "ix_netmacros.h"

/*
*******************************************************************************
* Shared Memory Macros (memory accessible by Acceleration Engines, e.g. QAT)
*******************************************************************************
*/

#define QAT_MEMORY (1)


/**
 *******************************************************************************
 * @ingroup QatalMem
 *	This macro rounds up a number to a be a multiple of the alignment when
 *	the alignment is a power of 2.
 *
 * @param[in] num   Number
 * @param[in] align Alignement (must be a power of 2)
 *
 ******************************************************************************/
#define QAT_ALIGN_POW2_ROUNDUP(num, align) \
    ( ((num) + (align) -1) & ~((align) - 1) )


/**
 *******************************************************************************
 * @ingroup QatalMem
 *	This macro uses an OSAL macro to convert from a physical address to a
 *	virtual address.
 *
 * @param[in] pPhysAddr   The address to be converted.
 *
 * @retval The converted virtual address
 ******************************************************************************
 *#define QAT_OS_PHYS_TO_VIRT(pPhysAddr) \
 *   IX_OSAL_OS_MMU_PHYS_TO_VIRT(pPhysAddr)
 */

/**
 *******************************************************************************
 * @ingroup QatalMem
 *	This function allocates a specified amount of memory which can be accessed
 * by both the host processor and the acceleration engines
 * (e.g. ME, QAT, MMP, etc).  Byte alignment for the memory block can be
 * specified.  The address returned is a virtual address for
 * use on the host processor.  Use the macro @ref QATAL_MEM_SHARED_VIRT_TO_PHYS()
 * to obtain a corresponding physical address for use on acceleration engines.
 *
 * @param ppMemAddr  IN/OUT the address of the pointer to which the allocated
 *			    memory will be assigned.
 * @param sizeBytes  IN     the amount of memory (in bytes) to allocate
 * @param alignShift IN     log base 2 of the required memory block byte
 *			    alignment (e.g. 3 for 8-byte alignment, 6 for
 *			    64-byte alignment)
 *
 * @retval CPA_STATUS_SUCCESS	    Macro executed successfully.
 * @retval CPA_STATUS_RESOURCE	     Macro failed to allocate memory.
 *
 * @note Assuming that this memory cannot be swapped out by the OS
 *
 ******************************************************************************/
static __inline CpaStatus
QatalMem_IoMemAlloc(
    void **ppMemAddr,
    Cpa32U sizeBytes )

{
    Cpa32U alignment = 0;
    Cpa32U alignShift = QATAL_64BYTE_ALIGNMENT_SHIFT;

    if (NULL == ppMemAddr)
    {
	return CPA_STATUS_RESOURCE;
    }

    alignment = 2 << (alignShift -1) ;

    *ppMemAddr = (Cpa8U *)ixOsalMemAllocAligned
				( 0,sizeBytes,alignment);

    if (NULL == *ppMemAddr)
    {
	return CPA_STATUS_RESOURCE;
    }


    return CPA_STATUS_SUCCESS;
}

#define QATAL_MEM_SHARED_ALLOC(ppMemAddr, sizeBytes) \
    QatalMem_IoMemAlloc((void *)(ppMemAddr),(sizeBytes))


static __inline CpaStatus
QatalMem_SpecialAlloc(
    void **ppMemAddr,
    Cpa32U sizeBytes )

{
    if (ppMemAddr == NULL)
    {
	return CPA_STATUS_FAIL;
    }

    *ppMemAddr = (Cpa8U *)ixOsalMemAllocAligned
				   ( 0,sizeBytes,(0x4000));

    if (*ppMemAddr == NULL)
    {
	 return CPA_STATUS_FAIL;
    }


    return CPA_STATUS_SUCCESS;
}

#define QATAL_MEM_SPECIAL_ALLOC(ppMemAddr, sizeBytes) \
    QatalMem_SpecialAlloc((void *)(ppMemAddr),(sizeBytes))


/**
 *******************************************************************************
 * @ingroup QatalMem
 *	This function frees a block of memory allocated using the function @ref
 * QatalMem_IoMemAlloc(), and resets the pointer to NULL.
 *
 * @param pMemAddr   IN  pointer to the memory block to be freed. If the pointer
 *			 is NULL, the function will exit silently.
 *
 * @note This macro expects a virtual address.
 *
 * @retval CPA_STATUS_SUCCESS	    Macro executed successfully.
 * @retval CPA_STATUS_FAIL	     Macro failed to free memory.
 ******************************************************************************/
static __inline CpaStatus
QatalMem_IoMemFree(
    void **ppAlignedMem)
{


    if (ppAlignedMem == NULL)
    {
       return CPA_STATUS_SUCCESS;
    }
    if (*ppAlignedMem == NULL)
    {
       return CPA_STATUS_SUCCESS;
    }

    ixOsalMemAlignedFree(*ppAlignedMem, 0);
    *ppAlignedMem = NULL;

    return CPA_STATUS_SUCCESS;
}

#define QATAL_MEM_SHARED_FREE(pMemAddr) \
    QatalMem_IoMemFree((void *)(pMemAddr))



static __inline CpaStatus
QatalMem_SpecialFree(
    void **ppMemAddr, int sizeBytes )

{


    if (ppMemAddr == NULL)
    {
       return CPA_STATUS_SUCCESS;
    }
    if (*ppMemAddr == NULL)
    {
       return CPA_STATUS_SUCCESS;
    }

    ixOsalMemAlignedFree(*ppMemAddr, 0);
    *ppMemAddr = NULL;

    return CPA_STATUS_SUCCESS;
}


#define QATAL_MEM_SPECIAL_FREE(ppMemAddr ,sizeBytes ) \
    QatalMem_SpecialFree((void *)(ppMemAddr), sizeBytes )

/**
 *******************************************************************************
 * @ingroup QatalMem
 *	This function provides a corresponding physical address for supplied
 * virtual address.  A physical address is typically required by the
 * acceleration engines to access a memory block provided by the host processor
 *
 * @param pVirtAddr IN	   virtual memory address to be translated
 * @param pPhysAddr IN/OUT address of variable where physical address will
 *			   be stored
 *
 * @retval CPA_STATUS_SUCCESS	    Macro executed successfully.
 * @retval CPA_STATUS_FAIL	     Macro failed to free memory.
 *
 * @note It is assumed that physical addresses are stored in 64-bit variables
 *
 ******************************************************************************/
 static __inline CpaStatus
   QatalMem_IoMemVirt2Phys(
     void *pVirtAddr,
    Cpa64U *pPhysAddr)
 {

    *pPhysAddr = IX_OSAL_OS_MMU_VIRT_TO_PHYS( pVirtAddr);

   return CPA_STATUS_SUCCESS;
}

 #define QATAL_MEM_SHARED_VIRT_TO_PHYS(pVirtAddr, pPhysAddr) \
   QatalMem_IoMemVirt2Phys((void *)(pVirtAddr), (pPhysAddr))


/**
 *******************************************************************************
 * @ingroup QatalMem
 *	This function provides a corresponding virtual address for supplied
 * physical address.  A virtual address is typically required by the host
 * processor to access a memory block shared with the acceleration engines
 *
 * @param physAddr   IN     physical memory address to be translated
 * @param ppVirtAddr IN/OUT address of pointer where virtual address will be
 *			    stored
 *
 * @retval CPA_STATUS_SUCCESS	    Macro executed successfully.
 * @retval CPA_STATUS_FAIL	     Macro failed to free memory.
 ******************************************************************************
 * static __inline CpaStatus
 * QatalMem_IoMemPhys2Virt(
 *   Cpa64U physAddr,
 *   void **ppVirtAddr)
 * {
 *
 *   *ppVirtAddr = (void *)IX_OSAL_OS_MMU_PHYS_TO_VIRT(physAddr);
 *   return CPA_STATUS_SUCCESS;
 * }
 *
 * #define QATAL_MEM_SHARED_PHYS_TO_VIRT(physAddr, ppVirtAddr) \
 *   QatalMem_IoMemPhys2Virt(physAddr, (void *)ppVirtAddr)
 */
 
/**
 *******************************************************************************
 * @ingroup QatalMem
 *	This macro verifies that the memory at the supplied virtual address
 * is shareable (i.e. accessible by the acceleration engines)
 *
 * @param virtAddr  IN	virtual memory address to be checked
 *
 * @retval CPA_STATUS_SUCCESS	    Memory is sharable.
 * @retval CPA_STATUS_FAIL	     Memory is not shareable (only IA can access)
 ******************************************************************************
 *#define QATAL_MEM_SHARED_ADDR_VERIFY(virtAddr)   \
 *   ((IX_SUCCESS != ix_rm_mem_is_virtaddr_valid( \
 *	 (void *)(virtAddr))			  \
 *   ) ? CPA_STATUS_FAIL : CPA_STATUS_SUCCESS)
 *
 *
 *
 */

/**
 *******************************************************************************
 * @ingroup QatalMem
 *	This macro can be used to write to a variable that will be read by the
 * QAT. The macro will automatically detect the size of the target variable and
 * will select the correct method for performing the write. The data is cast to
 * the type of the field that it will be written to.
 *
 * @param var  IN/OUT The variable to be written. Can be a field of a struct.
 *
 * @param data IN     The value to be written.	Will be cast to the size of the
 *		      target.
 *
 * @retval none
 ******************************************************************************/
#define QATAL_MEM_SHARED_WRITE(var, data)  \
do {					 \
    switch(sizeof(var)) 		 \
    {					 \
	case 1: 			 \
	    (var) = (Cpa8U)(data);     \
	    break;			 \
	case 2: 			 \
	    (var) = IX_HTON16((Cpa16U)(data));	  \
	    break;			 \
	case 4: 			 \
	    (var) = IX_HTON32((Cpa32U)(data));	  \
	    break;			 \
	case 8: 			 \
	    (var) = IX_HTON64((Cpa64U)(data));	  \
	    break;			 \
	default:			 \
	    break;			 \
    }					 \
} while(0)


/**
 *******************************************************************************
 * @ingroup QatalMem
 *	This macro can be used to read a variable that was written by the QAT.
 * The macro will automatically detect the size of the data to be read and will
 * select the correct method for performing the read. The value read from the
 * variable is cast to the size of the data type it will be stored in.
 *
 * @param var  IN     The variable to be read. Can be a field of a struct.
 *
 * @param data IN/OUT The variable to hold the result of the read. Data read will
 *		      be cast to the size of this variable
 *
 * @retval none
 ******************************************************************************/
#define QATAL_MEM_SHARED_READ(var, data) \
do {				       \
    switch(sizeof(var)) 	       \
    {				       \
	case 1: 		       \
	    (data) = (var);	       \
	    break;		       \
	case 2: 		       \
	    (data) = IX_NTOH16(var);   \
	    break;		       \
	case 4: 		       \
	    (data) = IX_NTOH32(var);   \
	    break;		       \
	case 8: 		       \
	    (data) = IX_NTOH64(var);   \
	    break;		       \
	default:		       \
	    break;		       \
    }				       \
} while(0)


/**
 *******************************************************************************
 * @ingroup QatalMem
 *	This macro can be used to write a pointer to a QAT request. The fields
 *	for pointers in the QAT request and response messages are always 64 bits.
 *
 * @param var  IN/OUT The variable to be written to. Can be a field of a struct.
 *
 * @param data IN     The value to be written.	Will be cast to size of target
 *		      variable
 *
 * @retval none
 ******************************************************************************/
/* cast pointer to scalar of same size of the native pointer */
#define QATAL_MEM_SHARED_WRITE_FROM_PTR(var, data) \
    ((var) = (Cpa64U)(unsigned long)(data))
/* Note: any changes to this macro implementation should also be made to the similar
 * QATAL_MEM_CAST_PTR_TO_UINT64 macro
 */


/**
 *******************************************************************************
 * @ingroup QatalMem
 *	This macro can be used to read a pointer from a QAT response. The fields
 *	for pointers in the QAT request and response messages are always 64 bits.
 *
 * @param var  IN     The variable to be read. Can be a field of a struct.
 *
 * @param data IN/OUT The variable to hold the result of the read. Data read
 *		      will be cast to the size of this variable
 *
 * @retval none
 ******************************************************************************/
/* Cast back to native pointer */
#define QATAL_MEM_SHARED_READ_TO_PTR(var, data) 		 \
    ((data) = (void *)(unsigned long)(var))


/**
 *******************************************************************************
 * @ingroup QatalMem
 *	This macro can be used to write an address variable that will be read by
 * the QAT.  The macro will perform the necessary virt2phys address translation
 *
 * @param var  IN/OUT The address variable to write. Can be a field of a struct.
 *
 * @param data IN     The pointer variable to containing the address to be
 *		      written
 *
 * @retval none
 ******************************************************************************/
#define QATAL_MEM_SHARED_WRITE_VIRT_TO_PHYS_PTR(var, pPtr)	 \
do {								 \
    Cpa64U physAddr = 0;				       \
    (void)QATAL_MEM_SHARED_VIRT_TO_PHYS((pPtr), &physAddr);	 \
    QATAL_MEM_SHARED_WRITE((var), physAddr);			 \
} while (0)

/**
 *******************************************************************************
 * @ingroup QatalMem
 *	This macro can be used to read an address variable that was written by
 * the QAT.  The macro will perform the necessary phys2virt address translation
 *
 * @param var  IN The address variable to read. Can be a field of a struct.
 *
 * @param data IN/OUT The pointer variable to hold the result of the read.
 *
 * @retval none
 *****************************************************************************
 * #define QATAL_MEM_SHARED_READ_PHYS_TO_VIRT_PTR(var, pPtr)	 \
 * do {								 \
 *   Cpa64U physAddr;					       \
 *   QATAL_MEM_SHARED_READ((var), physAddr);			 \
 *   (void)QATAL_MEM_SHARED_PHYS_TO_VIRT(physAddr, &(pPtr));	 \
 * } while (0)
 *
 */

/*
*******************************************************************************
* 32-bit / 64-bit Conversion Macros
*******************************************************************************
*/

/**
 *******************************************************************************
 * @ingroup QatalMem
 *	This macro safely casts a pointer to a Cpa64U type.
 *
 * @param pPtr	The pointer to be cast.
 * @retval pointer cast to Cpa64U
 ******************************************************************************/
#define QATAL_MEM_CAST_PTR_TO_UINT64(pPtr) \
    ((Cpa64U)(unsigned long)(pPtr))

/*
*******************************************************************************
* OS Memory Macros
*******************************************************************************
*/

/**
 *******************************************************************************
 * @ingroup QatalMem
 *	This function allocates the memory for the given size and stores the
 *	address of the memory allocated in the pointer.
 *
 * @param ppPtr IN/OUT	address of pointer where address will be stored
 * @param size	IN	the size of the memory to be allocated.
 *
 * @retval CPA_STATUS_RESOURCE	Macro failed to allocate Memory
 * @retval CPA_STATUS_SUCCESS  Macro executed successfully
 *
 ******************************************************************************/
static __inline CpaStatus
QatalMem_OsMemAlloc(void **ppMemAddr,
		  Cpa32U sizeBytes)
{
    *ppMemAddr = ixOsalMemAlloc(sizeBytes);
    if (NULL == *ppMemAddr)
    {
	return CPA_STATUS_RESOURCE;
    }

    return CPA_STATUS_SUCCESS;
}

#define QATAL_OS_MALLOC(ppMemAddr, sizeBytes) \
    QatalMem_OsMemAlloc((void *)(ppMemAddr), (sizeBytes))


/**
 *******************************************************************************
 * @ingroup QatalMem
 *	This macro allocates the memory for the given struct type and stores
 *	it in the given pointer. The memory is zero initialised.
 *
 * @param ppMemAddr IN/OUT the address of the pointer to store the allocated
 *			   memory address
 * @param sizeBytes IN	   the size of the memory to be allocated
 *
 * @retval CPA_STATUS_RESOURCE	Macro failed to allocate Memory
 * @retval CPA_STATUS_SUCCESS  Macro executed successfully
 *
 ******************************************************************************/
static __inline CpaStatus
QatalMem_OsMemCalloc(void **ppMemAddr,
		   Cpa32U sizeBytes)
{
    *ppMemAddr = ixOsalMemAlloc(sizeBytes);
    if (NULL == *ppMemAddr)
    {
	return CPA_STATUS_RESOURCE;
    }

    ixOsalMemSet(*ppMemAddr, 0, sizeBytes);
    return CPA_STATUS_SUCCESS;
}

#define QATAL_OS_CALLOC(ppPtr, sizeBytes) \
    QatalMem_OsMemCalloc((void *)(ppPtr), (sizeBytes))

#define QATAL_CALLOC QATAL_OS_CALLOC


/**
 *******************************************************************************
 * @ingroup QatalMem
 *	This macro frees the memory at the given address, and resets the pointer
 * to NULL
 *
 * @param ppMemAddr IN/OUT  address of pointer where mem address is stored.
 *			    If pointer is NULL, the function will exit silently
 *
 * @retval CPA_STATUS_RESOURCE	Macro failed to allocate Memory
 * @retval CPA_STATUS_SUCCESS  Macro executed successfully
 *
 ******************************************************************************/
static __inline void
QatalMem_OsMemFree(void **ppMemAddr)
{
    if (NULL != *ppMemAddr)
    {
	ixOsalMemFree(*ppMemAddr);
	*ppMemAddr = NULL;
    }
}

#define QATAL_OS_FREE(pMemAddr) \
    QatalMem_OsMemFree((void *)&pMemAddr)


/* Deprecated macros */
#define QATAL_FREE_WITH_CHECK QATAL_OS_FREE
#define QATAL_FREE	      QATAL_OS_FREE


/*
*******************************************************************************
* Memory Re-alignment Functions
*******************************************************************************
*/

/**
 * @ingroup QatalMem
 * @brief Type definition to control copying between two different size
 *	  memory areas.
 * @description
 *	  Type definition to control copying between two different size
 *	  memory areas. Right padding is similar to memcpy; bytes are copied
 *	  from the beginning of the source area to the beginning of the
 *	  destination area. If the destination is smaller than the source, the
 *	  data is truncated. If the destination is larger than the source, there
 *	  will be padding remaining in the right side of the destination.
 *
 *	  Left padding is used for copying large numbers. Copying is done from
 *	  the end of the source buffer to the end of the destination buffer,
 *	  working back to the start of the buffers. If the source buffer is
 *	  larger than the destination buffer, it is truncated. If the
 *	  destination is larger than the source, there will be padding remaining
 *	  in the left side (beginning) of the destination.
 *
 *	  Bitwise OR your padding choice with ICP_QATAL_MEM_PAD_ZEROES to force
 *	  the alignment/restore functions to overwrite any padded areas with
 *	  zeroes.
 *
 * Examples:
 *
 * ICP_QATAL_MEM_PAD_RIGHT
 *
 *	source		    [abcdefghijk]
 *	larger destination  [abcdefghijk????]
 *	larger destination  [abcdefghijk0000] if ICP_QATAL_MEM_PAD_ZEROES is used
 *	smaller destination [abcdef]
 *
 * ICP_QATAL_MEM_PAD_LEFT
 *
 *	source		    [abcdefghijk]
 *	larger destination  [????abcdefghijk]
 *	larger destination  [0000abcdefghijk] if ICP_QATAL_MEM_PAD_ZEROES is used
 *	smaller destination [cdefghijk]
 */
typedef enum
{
    ICP_QATAL_MEM_DEFAULT    = QATAL_BIT(0),
    ICP_QATAL_MEM_NO_COPY    = QATAL_BIT(0),
    ICP_QATAL_MEM_PAD_RIGHT  = QATAL_BIT(1),
    ICP_QATAL_MEM_PAD_LEFT   = QATAL_BIT(2),
    ICP_QATAL_MEM_PAD_ZEROES = QATAL_BIT(15)
} icp_qatal_mem_padding_t;

#define QATAL_PTR unsigned long

#define QATAL_MEM_SET(pMemAddr,byteVal,byteSz) memset(pMemAddr,byteVal,byteSz)

#define QATAL_MEM_CPY(arg_pDest,arg_pSrc,arg_Count) memcpy(arg_pDest,arg_pSrc,arg_Count)

#endif /* QATAL_MEM_H */
