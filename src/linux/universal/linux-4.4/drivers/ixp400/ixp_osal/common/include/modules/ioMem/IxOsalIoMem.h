/*
 * @file        IxOsalIoMem.h
 * @author 	Intel Corporation
 * @date        25-08-2004
 *
 * @brief       description goes here
 */

/**
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
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
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
 */

#ifndef IxOsalIoMem_H
#define IxOsalIoMem_H


/*
 * Decide OS and Endianess, such as IX_OSAL_VXWORKS_LE.
 */
#include "IxOsalEndianess.h"

/**
 * @defgroup IxOsalIoMem Osal IoMem module
 *
 * @brief I/O memory and endianess support.
 *
 * @{ 
 */


/* Low-level conversion macros - DO NOT USE UNLESS ABSOLUTELY NEEDED */
#ifdef __wince

#define IX_OSAL_SWAP_LONG(wData)        ((((UINT32)wData << 24) | ((UINT32)wData >> 24)) | (((wData << 8) & 0xff0000) | ((wData >> 8) & 0xff00)))

#else

#define IX_OSAL_SWAP_LONG(wData)	IX_OSAL_OEM_SWAP_LONG(wData)

#endif /* ndef __wince */

#define IX_OSAL_SWAP_SHORT(sData)         ((sData >> 8) | ((sData & 0xFF) << 8))
#define IX_OSAL_SWAP_SHORT_ADDRESS(sAddr) ((sAddr) ^ 0x2)
#define IX_OSAL_SWAP_BYTE_ADDRESS(bAddr)  ((bAddr) ^ 0x3)


/*
 * Decide SDRAM mapping, then implement read/write
 */
#include "IxOsalMemAccess.h"


/**
 * @ingroup IxOsalIoMem
 * @enum IxOsalMapEntryType
 * @brief This is an emum for OSAL I/O mem map type. 
 */
typedef enum
{
    IX_OSAL_STATIC_MAP = 0,	 /**<Set map entry type to static map */
    IX_OSAL_DYNAMIC_MAP	     /**<Set map entry type to dynamic map */
} IxOsalMapEntryType;


/**
 * @ingroup IxOsalIoMem
 * @enum IxOsalMapEndianessType
 * @brief This is an emum for OSAL I/O mem Endianess and Coherency mode.
 */
typedef enum
{
    IX_OSAL_BE = 0x1,      /**<Set map endian mode to Big Endian */
    IX_OSAL_LE_AC = 0x2,   /**<Set map endian mode to Little Endian, Address Coherent */
    IX_OSAL_LE_DC = 0x4,   /**<Set map endian mode to Little Endian, Data Coherent */
    IX_OSAL_LE = 0x8       /**<Set map endian mode to Little Endian without specifying coherency mode */
} IxOsalMapEndianessType;


/**
 * @struct IxOsalMemoryMap 
 * @brief IxOsalMemoryMap structure
 */
typedef struct _IxOsalMemoryMap
{
    IxOsalMapEntryType type;   /**< map type - IX_OSAL_STATIC_MAP or IX_OSAL_DYNAMIC_MAP */

    UINT32 physicalAddress;    /**< physical address of the memory mapped I/O zone */

    UINT32 size;               /**< size of the map */


    UINT32 virtualAddress;     /**< virtual address of the zone; must be predefined 
                                    in the global memory map for static maps and has 
                                    to be NULL for dynamic maps (populated on allocation) 
								*/
    /*
     * pointer to a map function called to map a dynamic map; 
     * will populate the virtualAddress field 
     */
    void (*mapFunction) (struct _IxOsalMemoryMap * map);   /**< pointer to a map function called to map a dynamic map */

    /*
     * pointer to a map function called to unmap a dynamic map; 
     * will reset the virtualAddress field to NULL 
     */
    void (*unmapFunction) (struct _IxOsalMemoryMap * map); /**< pointer to a map function called to unmap a dynamic map */

    /*
     * reference count describing how many components share this map; 
     * actual allocation/deallocation for dynamic maps is done only 
     * between 0 <=> 1 transitions of the counter 
     */
    UINT32 refCount;   /**< reference count describing how many components share this map */

    /*
     * memory endian type for the map; can be a combination of IX_OSAL_BE (Big 
     * Endian) and IX_OSAL_LE or IX_OSAL_LE_AC or IX_OSAL_LE_DC
     * (Little Endian, Address Coherent or Data Coherent). Any combination is
     * allowed provided it contains at most one LE flag - e.g.
     * (IX_OSAL_BE), (IX_OSAL_LE_AC), (IX_OSAL_BE | IX_OSAL_LE_DC), (IX_OSAL_LE) 
     * are valid combinations while (IX_OSAL_BE | IX_OSAL_LE_DC | IX_OSAL_LE_AC) 
     * is not. 
     */
    IxOsalMapEndianessType mapEndianType; /**< memory endian type for the map */

    char *name;      /**< user-friendly name */
} IxOsalMemoryMap;




/* Internal function to map a memory zone
 * NOTE - This should not be called by the user.
 * Use the macro IX_OSAL_MEM_MAP instead
 */
PUBLIC void *ixOsalIoMemMap (UINT32 requestedAddress,
			     UINT32 size,
			     IxOsalMapEndianessType requestedCoherency);


/* Internal function to unmap a memory zone mapped with ixOsalIoMemMap
 * NOTE - This should not be called by the user.
 * Use the macro IX_OSAL_MEM_UNMAP instead
 */
PUBLIC void ixOsalIoMemUnmap (UINT32 requestedAddress, UINT32 coherency);

#if !defined(__linux_user) && !defined(__freebsd_user)

/* Internal function to convert virtual address to physical address 
 * NOTE - This should not be called by the user.
 * Use the macro IX_OSAL_MMAP_VIRT_TO_PHYS */
PUBLIC UINT32 ixOsalIoMemVirtToPhys (UINT32 virtualAddress, UINT32 coherency);


/* Internal function to convert physical address to virtual address 
 * NOTE - This should not be called by the user.
 * Use the macro IX_OSAL_MMAP_PHYS_TO_VIRT */
PUBLIC UINT32
ixOsalIoMemPhysToVirt (UINT32 physicalAddress, UINT32 coherency);

#endif /* linux_user || freebsd_user */

/**
 * @ingroup IxOsalIoMem
 *
 * @def IX_OSAL_MEM_MAP(physAddr, size)
 *
 * @brief Map an I/O mapped physical memory zone to virtual zone and return virtual 
 *        pointer.
 * @param  physAddr - the physical address
 * @param  size     - the size
 * @return start address of the virtual memory zone.
 * 
 * @note  This function maps an I/O mapped physical memory zone of the given size
 * into a virtual memory zone accessible by the caller and returns a cookie - 
 * the start address of the virtual memory zone. 
 * IX_OSAL_MMAP_PHYS_TO_VIRT should NOT therefore be used on the returned 
 * virtual address.
 * The memory zone is to be unmapped using IX_OSAL_MEM_UNMAP once the caller has
 * finished using this zone (e.g. on driver unload) using the cookie as 
 * parameter.
 * The IX_OSAL_READ/WRITE_LONG/SHORT macros should be used to read and write 
 * the mapped memory, adding the necessary offsets to the address cookie.
 */
#define IX_OSAL_MEM_MAP(physAddr, size) \
    ixOsalIoMemMap((physAddr), (size), IX_OSAL_COMPONENT_MAPPING)


/**
 * @ingroup IxOsalIoMem
 *
 * @def IX_OSAL_MEM_UNMAP(virtAddr)
 *
 * @brief Unmap a previously mapped I/O memory zone using virtual pointer obtained 
 *        during the mapping operation.
 *        pointer.
 * @param  virtAddr - the virtual pointer to the zone to be unmapped.
 * @return none
 * 
 * @note  This function unmaps a previously mapped I/O memory zone using
 * the cookie obtained in the mapping operation. The memory zone in question
 * becomes unavailable to the caller once unmapped and the cookie should be
 * discarded.
 *
 * This function cannot fail if the given parameter is correct and does not
 * return a value.
 */
#define IX_OSAL_MEM_UNMAP(virtAddr) \
    ixOsalIoMemUnmap ((virtAddr), IX_OSAL_COMPONENT_MAPPING)

#if !defined(__linux_user) && !defined(__freebsd_user)

/**
 * @ingroup IxOsalIoMem
 *
 * @def IX_OSAL_MMAP_VIRT_TO_PHYS(virtAddr)
 *
 * @brief This function Converts a virtual address into a physical 
 * address, including the dynamically mapped memory.
 *
 * @param  virtAddr - virtual address to convert
 * Return value: corresponding physical address, or NULL 
 */
#define IX_OSAL_MMAP_VIRT_TO_PHYS(virtAddr) \
    ixOsalIoMemVirtToPhys(virtAddr, IX_OSAL_COMPONENT_MAPPING)


/**
 * @ingroup IxOsalIoMem
 *
 * @def IX_OSAL_MMAP_PHYS_TO_VIRT(physAddr)
 *
 * @brief  This function Converts a virtual address into a physical 
 * address, including the dynamically mapped memory.
 *
 * @param  physAddr - physical address to convert
 * Return value: corresponding virtual address, or NULL 
 *
 */
#define IX_OSAL_MMAP_PHYS_TO_VIRT(physAddr) \
    ixOsalIoMemPhysToVirt(physAddr, IX_OSAL_COMPONENT_MAPPING)

#endif /* linux_user || freebsd_user */

/**
 * @} IxOsalIoMem
 */

#endif /* IxOsalIoMem_H */
