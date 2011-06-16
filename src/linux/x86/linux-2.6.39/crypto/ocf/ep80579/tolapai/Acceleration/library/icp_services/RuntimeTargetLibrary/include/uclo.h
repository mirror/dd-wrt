/**
 **************************************************************************
 * @file uclo.h
 *
 * @description
 *      This is the header file for uCode Loader Library
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
 * @file uclo.h
 * 
 * @defgroup Uclo Microcode Loader Library
 *
 * @description
 *      This header file that contains the prototypes and definitions required
 *      for microcode loader
 *
 *****************************************************************************/

#ifndef __UCLO_H__
#define __UCLO_H__

#include "icptype.h"
#include "core_platform.h"
#include "uof.h"
#include "dbgAeInfo.h"
#include "uclo_status.h"

#define UCLO_MAX_AES    0x18        /**< maximum number of AE's */
#define UCLO_BADFUNCID  -1          /**< bad function ID */

typedef struct UcLo_VarUmemSeg_S{
    unsigned int umemBaseAddr;      /**< ustore memory segment base address */
    unsigned int umemSize;          /**< ustore memory segment byte size */
}UcLo_VarUmemSeg_T;

typedef struct UcLo_VarMemSeg_S{
    unsigned int sram0BaseAddr;     /**< SRAM0 memory segment base address */
    unsigned int sram0Size;         /**< SRAM0 memory segment byte size */
    unsigned int sram0Alignment;    /**< SRAM0 memory segment byte alignment */
    unsigned int sdramBaseAddr;     /**< DRAM0 memory segment base address */
    unsigned int sdramSize;         /**< DRAM0 memory segment byte size */
    unsigned int sdramAlignment;    /**< DRAM0 memory segment byte alignment */
    unsigned int sdram1BaseAddr;    /**< DRAM1 memory segment base address */
    unsigned int sdram1Size;        /**< DRAM1 memory segment byte size */
    unsigned int sdram1Alignment;   /**< DRAM1 memory segment byte alignment */
    unsigned int scratchBaseAddr;   /**< SCRATCH memory segment base address */
    unsigned int scratchSize;       /**< SCRATCH memory segment byte size */
    unsigned int scratchAlignment;  /**< SCRATCH memory segment byte alignment */
    UcLo_VarUmemSeg_T meUmemSeg[UCLO_MAX_AES]; /**< ustore being used in the uof */
                                   /**< ustore values are specified in terms of
                                      uwords, not bytes or 32-bit words.
                                      Array is indexed using physical AE addrs */
}UcLo_VarMemSeg_T;

typedef struct UcLo_SymbolInfo_S{
    uof_MemRegion memType;     /**< memory type: sram/dram/lmem/umem/scratch */
    unsigned int baseAddr;     /**< word aligned byte address of the symbol */ 
    unsigned int allocSize;    /**< total bytes allocated */
}UcLo_SymbolInfo_T;

typedef struct{
    unsigned int assignedCtxMask; /**< image assigned context mask */
    char        *imageName;       /**< image name */
}ucLo_ImageAssign_T;

typedef struct{
   unsigned int    value;       /**< value of import variable */
   unsigned int    valueAttrs;  /**< import variable attribute 
                                     bit<0> (Scope: 0=global), 
                                     bit<1> (init: 0=no, 1=yes) */
}uclo_ImportValue_T;

#ifdef __cplusplus
extern "C"{
#endif

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      This function initializes the uclo library and performs any AccelEngine
 *      driver initialization. It should be called prior to calling any of the 
 *      other functions in the uclo library
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param  - none
 *
 * @retval - none
 * @retval - none
 * 
 * 
 *****************************************************************************/

EXPORT_API void UcLo_InitLib(void);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Initializes the Loader library. Microengines whose corresponding bit 
 *      is set in aeMask are set to the powerup default state. The library 
 *      assumes that those AccelEngines that are not specified are in a
 *      reset state
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param aeMask           IN An integer mask specifying the AccelEngines to 
 *                            initialize
 *
 * @retval - none
 * @retval - none
 * 
 * 
 *****************************************************************************/
 
EXPORT_API void UcLo_InitLibUeng(unsigned int aeMask);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Writes a 32-bit unsigned value to the specified AccelEngine(s) and 
 *      micro address
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - IN A void pointer reference to the loaded and mapped object
 * @param aeMask - IN An integer mask specifying the AccelEngines to which to
 *                    write the value uWord at the address uAddr
 * @param phyUaddr - IN An integer value indicating the microstore physical  
 *                      address to write the value to
 * @param uWord - IN The value to be written to one or more AccelEngines 
 *                   initialize
 *
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_BADARG Invalid argument specified
 * @retval UCLO_FAILURE Operation failed
 * 
 * 
 *****************************************************************************/

EXPORT_API int UcLo_WriteUword(void *handle, 
                               unsigned int aeMask, 
                               unsigned int phyUaddr, 
                               uword_T uWord);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Reads a word from the specified uEngine
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - IN A void pointer reference to the loaded and mapped object
 * @param ae - IN Specifies a AccelEngine
 * @param phyUaddr - IN An integer value indicating the microstore physical  
 *                      address to write the value to
 * @param uWord - OUT The value read from the specified AccelEngines at address 
 *                    uAddr 
 *
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_BADARG Invalid argument specified
 * @retval UCLO_FAILURE Operation failed
 * 
 * 
 *****************************************************************************/

EXPORT_API int UcLo_ReadUword(void *handle, 
                              unsigned char ae, 
                              unsigned int phyUaddr, 
                              uword_T *uWord);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Compares the UOF image that was either loaded or mapped to the content 
 *      of the assigned AccelEngine(s)
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - IN A void pointer reference to the loaded and mapped object
 * @param ucodeImageName - IN Pointer to character string containing name of 
 *                             the AccelEngine image
 *
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_NOOBJ No object was either loaded or mapped
 * @retval UCLO_IMGNOTFND Image not found
 * @retval UCLO_FAILURE Operation failed
 * 
 * 
 *****************************************************************************/

EXPORT_API int UcLo_VerifyUimage(void *handle, char *ucodeImageName);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Compares the content of the specified AccelEngine to its assigned UOF 
 *      image that was either mapped or loaded
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - IN A void pointer reference to the loaded and mapped object
 * @param ae - IN Specifies a AccelEngine
 * 
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_NOOBJ No object was either loaded or mapped
 * @retval UCLO_IMGNOTFND Image not found
 * @retval UCLO_FAILURE Operation failed
 * @retval UCLO_BADARG Invalid argument specified
 * 
 * 
 *****************************************************************************/

EXPORT_API int UcLo_VerifyUengine(void *handle, unsigned char ae);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Writes the specified image in the UOF object to the assigned 
 *      AccelEngines 
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - IN A void pointer reference to the loaded and mapped object
 * @param ae - IN Specifies a AccelEngine
 * 
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_NOOBJ No object was either loaded or mapped
 * @retval UCLO_IMGNOTFND Image not found
 * @retval UCLO_FAILURE Operation failed
 * @retval UCLO_BADARG Invalid argument specified
 * @retval UCLO_UNINITVAR A variable was not initialized
 * 
 * 
*****************************************************************************/

EXPORT_API int UcLo_WriteUimage(void *handle, char *ucodeImageName);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Writes all the microcode images to one or more appropriate 
 *      AccelEngine(s)
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - IN A void pointer reference to the loaded and mapped object
 * 
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_NOOBJ No object was either loaded or mapped
 * @retval UCLO_IMGNOTFND Image not found
 * @retval UCLO_FAILURE Operation failed
 * @retval UCLO_BADARG Invalid argument specified
 * 
 * 
 *****************************************************************************/

EXPORT_API int UcLo_WriteUimageAll(void *handle);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Retrieve the locations, sizes, and alignment of the C compiler variables
 *      memory segments 
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - IN A void pointer reference to the loaded and mapped object
 * @param varMemSeg - OUT A pointer to variable segment structure
 * 
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_BADARG Invalid argument specified
 * @retval UCLO_NOOBJ No object was either loaded or mapped
 * 
 * 
 *****************************************************************************/

EXPORT_API int UcLo_GetVarMemSegs(void *handle, UcLo_VarMemSeg_T *varMemSeg);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Removes all references to the UOF image -- if one was either loaded 
 *      or mapped. The memory region of mapped UOF, by calling 
 *      UcLo_MapImageAddr, is not deallocated by this function and it is 
 *      responsible of the caller to explicitly delete it
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - IN A void pointer reference to the loaded and mapped object
 *
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_BADARG Invalid argument specified
 * @retval UCLO_FAILURE Operation failed
 * 
 * 
 *****************************************************************************/

EXPORT_API int UcLo_DeleObj(void *handle);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Maps the memory location pointed to by the addrPtr parameter to the 
 *      microcode Object File (UOF). The library verifies that the mapped 
 *      object fits within the size specified by the memSize parameter and 
 *      that the object checksum is valid.
 *      If the readOnly parameter is not equal to zero, then the library copies
 *      any region of memory that it needs to modify. Also, the memory region, 
 *      (addrPtr through addrPtr + memSize + 1), should not be modified by the
 *      caller while the image is mapped to the region -- unless through the 
 *      use of uclo library functions.
 *      The user should call UcLo_DeleObj() to remove reference to the object 
 *      and to free the resources that was allocated by the library.
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - IN A void pointer reference to the loaded and mapped object
 * @param addrPtr - IN Pointer to the memory location where the Ucode Object 
 *                     File image resides
 * @param memSize - IN An integer indicating the size of the memory region 
 *                     pointed to by the addrPtr parameter  
 * @param readOnly - IN Indicates whether the memory space being pointed to by
 *                       addrPtr is read-only initialize
 *
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_NOOBJ No object was either loaded or mapped
 * 
 * 
 *****************************************************************************/

EXPORT_API int UcLo_MapObjAddr(void **handle, 
                               void *addrPtr, 
                               int memSize, 
                               int readOnly);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Loads the UOF objects from the file specified by fileName to a buffer. 
 *      If UCLO_SUCCESS is returned, then a buffer is allocated and its pointer
 *      and size are returned in objBuf and chunkSize respectively. It is the 
 *      responsibility of the caller to delete the buffer. 
 *      This API does not support kernel mode as it involves in file operation.
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param fileName - IN Character string pointer to the name of the image file 
 *                      that was created by linker.
 * @param objBuf - OUT A char pointer to accept the pointer to the object
 * @param chunkSize - OUT The size of the buffer pointed to by objBuf
 *
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_FILEFAIL File operation failed due to file related error
 * @retval UCLO_BADARG Invalid argument specified
 * 
 * 
 *****************************************************************************/

#ifndef __KERNEL__

EXPORT_API int UcLo_CopyObjFile(char *fileName, 
                                char **objBuf, 
                                unsigned int *chunkSize);

#endif /* #ifndef (__KERNEL__) */

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Loads an object file that was created by the microcode linker to 
 *      processor memory. The library allocates and manages the resources to 
 *      accommodate the object. The user should call UcLo_DeleObj() to remove 
 *      reference to the object and to free the resources that was allocated by 
 *      the library.
 *      This API does not support kernel mode as it involves in file operation.
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - OUT A void pointer reference to the loaded object
 * @param fileName - IN Character string pointer to the name of the image file 
 *                      that was created by linker
 *
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_FILEFAIL File operation failed due to file related error
 * @retval UCLO_BADARG Invalid argument specified
 * @retval UCLO_MEMFAIL Failed allocate memory
 * @retval UCLO_BADOBJ Error in object format or checksum
 * @retval UCLO_UOFVERINCOMPAT The UOF is incompatible with this version of 
 *                             the Loader library
 * @retval UCLO_UOFINCOMPAT The UOF is incompatible with this version of the 
                            chip
 * 
 * 
 *****************************************************************************/

#ifndef __KERNEL__

EXPORT_API int UcLo_LoadObjFile(void **handle, char *fileName);

#endif /* #ifndef (__KERNEL__) */

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Creates an association between an application value and a microcode 
 *      symbol. It initializes all occurrences of the specified symbol in the 
 *      UOF image to the 32-bit value, or portion of the 32-bit value, as 
 *      defined by the ucode assembler (uca). 
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - IN A void pointer reference to the loaded and mapped object
 * @param ucodeImageName - IN Pointer to a character string containing the name
 *                            of the AccelEngine image
 * @param ucodeSymName - IN String pointer to the name of the microcode symbol 
 *                          to bind 
 * @param value - IN An unsigned 32-bit value of which to initialize the 
 *                   microcode symbols 
 *
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_BADARG Invalid argument specified
 * @retval UCLO_NOOBJ No object was either loaded or mapped
 * @retval UCLO_IMGNOTFND Image not found
 * @retval UCLO_SYMNOTFND Symbol not found
 * 
 * 
 *****************************************************************************/

EXPORT_API int UcLo_BindSymbol(void *handle, 
                               char *ucodeImageName, 
                               char *ucodeSymName, 
                               int value);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Binds a value to the microcode symbol in the UOF image. Because 
 *      multiple AccelEngines may be assigned to the same UOF image, the 
 *      result of this function applies to the specified AccelEngine
 *      and to all assigned AccelEngines
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - IN A void pointer reference to the loaded and mapped object
 * @param ae - IN Specifies a AccelEngine
 * @param ucodeSymName - IN String pointer to the name of the microcode symbol 
 *                          to bind 
 * @param value - IN An unsigned 32-bit value of which to initialize the 
 *                   microcode symbols 
 *
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_BADARG Invalid argument specified
 * @retval UCLO_NOOBJ No object was either loaded or mapped
 * @retval UCLO_IMGNOTFND Image not found
 * @retval UCLO_SYMNOTFND Symbol not found
 * 
 * 
 *****************************************************************************/

EXPORT_API int UcLo_AeBindSymbol(void *handle, 
                                 unsigned char ae, 
                                 char *ucodeSymName, 
                                 int value);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      First, the function verifies that the file is an UOF. If it is, then 
 *      the UOF format version of the file is returned in the minVer and majVer
 *      arguments.
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param fileHdr - IN A handle to the file 
 * @param minVer - IN The UOF format minor version
 * @param majVer - IN The UOF format major version
 *
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_UOFINCOMPAT The UOF is incompatible with this version of 
 *                          the Loader library
 * @retval UCLO_FAILURE Operation failed
 * 
 * 
 *****************************************************************************/

EXPORT_API int UcLo_verifyFile(char *fileHdr, 
                               short *minVer, 
                               short *majVer);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Returns the specified object's CRC checksum
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - IN A void pointer reference to the loaded and mapped object
 * @param checksum - OUT The specified object's checksum
 *
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_BADARG Invalid argument specified
 * @retval UCLO_NOOBJ No object was either loaded or mapped
 * 
 * 
 *****************************************************************************/

EXPORT_API int UcLo_GetChecksum(void *handle, unsigned int *checksum);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Return the image-name by name of the specified AE. The return
 *      value to the image-name must not be modified micro address
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - IN A void pointer reference to the loaded and mapped object
 * @param ae - IN Specifies a AccelEngine
 *
 * @retval String pointer to the name of the image
 * 
 * 
 *****************************************************************************/

EXPORT_API char *UcLo_GetAeImageName(void *handle, unsigned char ae);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Returns the mask specifying the AccelEngines that are assigned to an 
 *      image in the UOF
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - IN A void pointer reference to the loaded and mapped object
 *
 * @retval Mask of the assigned AEs
 * 
 * 
 *****************************************************************************/

EXPORT_API unsigned int UcLo_GetAssignedAEs(void *handle);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Returns the mask specifying the contexts that are assigned to the 
 *      AccelEngine in the UOF 
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - IN A void pointer reference to the loaded and mapped object
 * @param ae - IN Specifies a AccelEngine
 *
 * @retval Mask of the assigned contexts
 * 
 * 
 *****************************************************************************/

EXPORT_API unsigned int UcLo_GetAssignedCtxs(void *handle, unsigned char ae);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Retrieves the udbug information from debug section of an UOF. If the 
 *      UOF is linked with debug information, then this function allocates 
 *      memory, extracts the debug information, and returns a pointer in the 
 *      image parameter. It is the caller's responsibility to delete the debug 
 *      information. 
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - IN A void pointer reference to the loaded and mapped object
 * @param image - OUT An pointer reference to the debug information
 *
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_BADARG Invalid argument specified
 * @retval UCLO_FAILURE Operation failed
 * @retval UCLO_NOOBJ No object was either loaded or mapped
 * 
 * 
 *****************************************************************************/

EXPORT_API int UcLo_GetDebugInfo(void *handle, dbgAeInfo_Image_T *image);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Reads the specified file and extracts information to be used to 
 *      initialize import-variables. The file format is the following 
 *      space-separated values:
 *      <chipName> <imageName> <symbolName> <value>
 *      ImageName and chipName can be specified as a wildcard (*) to match any 
 *      chip or image. If imagename is *, then all instances of the variable 
 *      found in any of the images are initialized to the specified value.
 *      This API does not support kernel mode as it involves in file operation.
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param filename - IN Character string pointer to the name of the Ivd file 
 *
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_FAILURE Operation failed
 * 
 * 
 *****************************************************************************/

#ifndef __KERNEL__

EXPORT_API int UcLo_LoadIvdFile(char *filename);

#endif /* #ifndef (__KERNEL__) */

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Extracts the information from string buffer to be used to initialize 
 *      import-variables.
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param ivdBuf - IN Character string pointer to the buffer
 * @param ivdBufLen - IN Buffer string length
 *
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_FAILURE Operation failed
 * 
 * 
 *****************************************************************************/

EXPORT_API int UcLo_LoadIvdBuf(char *ivdBuf, unsigned int ivdBufLen);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Removes all the information and resources that were created as a 
 *      result of calls to UcLo_LoadIvdFile, or UcLo_LoadIvdBuf
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param - none
 *
 * @retval - none
 * 
 * 
 *****************************************************************************/

EXPORT_API void UcLo_DelIvd(void);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Retrieves the storage information of a local symbol
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - IN A void pointer reference to the loaded and mapped object
 * @param ae - IN Specifies a AccelEngine
 * @param symName - IN String pointer to the name of the microcode local symbol 
 *                     to bind 
 * @param symInfo - OUT A pointer reference to the symbol information
 *
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_BADARG Invalid argument specified
 * @retval UCLO_FAILURE Operation failed
 * @retval UCLO_NOOBJ No object was either loaded or mapped
 * 
 * 
 *****************************************************************************/

EXPORT_API int UcLo_GetLocalSymbolInfo(void *handle, 
                                       unsigned int ae, 
                                       char *symName, 
                                       UcLo_SymbolInfo_T *symInfo);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Retrieves the storage information of a global symbol
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - IN A void pointer reference to the loaded and mapped object
 * @param symName - IN String pointer to the name of the microcode global symbol 
 *                     to bind 
 * @param symInfo - OUT A pointer reference to the symbol information
 *
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_BADARG Invalid argument specified
 * @retval UCLO_FAILURE Operation failed
 * @retval UCLO_NOOBJ No object was either loaded or mapped
 * 
 * 
 *****************************************************************************/

EXPORT_API int UcLo_GetGlobalSymbolInfo(void *handle, 
                                        char *symName, 
                                        UcLo_SymbolInfo_T *symInfo);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Retrieve the application meta-data asociated with the AE or any
 *      AE if hwAe is UCLO_BADAE.
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - IN A void pointer reference to the loaded and mapped object
 * @param ae - IN Specifies a AccelEngine
 *
 * @retval Pointer to character string containing meta-data
 * 
 * 
 *****************************************************************************/

EXPORT_API char *UcLo_GetAppMetaData(void *handle, unsigned int ae);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Retrieves the physical ustore address for the specified virtual 
 *      ustore address
 * 
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - IN A void pointer reference to the loaded and mapped object
 * @param hwAe - IN Specifies a AccelEngine
 * @param virtUaddr - IN Virtual ustore address 
 * @param pageNum - IN Page number associated with the physical address
 * @param phyUaddr - OUT Physical ustore address
 *
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_ADDRNOTFOUND Address not found
 * @retval UCLO_BADARG Invalid argument specified
 * 
 * 
 *****************************************************************************/

EXPORT_API int UcLo_GetPhyUaddr(void *handle, 
                                unsigned int hwAe, 
                                unsigned int virtUaddr,
					            unsigned int*pageNum, 
					            unsigned int *phyUaddr);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Locate an import variable and returns its value and an attribute 
 *      indicating whether or not the variable was initialized, and whether 
 *      its scope is global or local
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param handle - IN A void pointer reference to the loaded and mapped object
 * @param hwAe - IN An integer mask specifying the AccelEngines to which to
 *                    write the value uWord at the address uAddr.
 * @param varName - IN Name of the import-variable 
 * @param importVal - OUT Pointer to structure to receive the import-variable's
 *                    value and attributes
 *
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_FAILURE Operation failed
 * @retval UCLO_BADARG Invalid argument specified
 * @retval UCLO_NOOBJ No object was either loaded or mapped
 * 
 * 
 *****************************************************************************/

EXPORT_API int UcLo_GetImportVal(void *handle, 
                                 unsigned int hwAe, 
                                 char *varName, 
                                 uclo_ImportValue_T *importVal);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Get the none-coherent dram base address
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param - none
 *
 * @retval none-coherent dram base address
 * 
 * 
 *****************************************************************************/

EXPORT_API unsigned int UcLo_Get_MENCBASE(void);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Set the SCRATCH/SRAM/NCDRAM/CDRAM allocated base address for AccelEngine
 *      
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param scratchOffset - IN Scratch memory address offset     
 * @param sramOffset - IN SRAM memory address offset
 * @param ncdramOffset - IN non-coherent DRAM memory address offset
 * @param cdramOffset - IN coherent DRAM memory address offset
 *
 * @retval UCLO_SUCCESS Operation was successful
 * @retval UCLO_FAILURE Operation failed
 * 
 * 
 *****************************************************************************/

EXPORT_API int UcLo_SetMemoryStartOffset(unsigned int scratchOffset, 
                                         unsigned int sramOffset, 
                                         unsigned int ncdramOffset, 
                                         unsigned int cdramOffset);

/**
 *****************************************************************************
 * @ingroup Uclo
 * 
 * @description
 *      Get the error string for provided error code
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param errCode - IN Loader library error code
 *
 * @retval Pointer to character string containing error description
 * 
 * 
 *****************************************************************************/

EXPORT_API char *UcLo_GetErrorStr(int errCode);

#ifdef __cplusplus
}
#endif

#endif          /* __UCLO_H__ */
