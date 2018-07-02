/**
 * @file IxNpeDlImageMgr_p.h
 *
 * @author Intel Corporation
 * @date 14 December 2001

 * @brief This file contains the private API for the ImageMgr module
 *
 * 
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

/**
 * @defgroup IxNpeDlImageMgr_p IxNpeDlImageMgr_p
 *
 * @brief The private API for the IxNpeDl ImageMgr module
 * 
 * @{
 */

#ifndef IXNPEDLIMAGEMGR_P_H
#define IXNPEDLIMAGEMGR_P_H


/*
 * Put the user defined include files required.
 */
#include "IxNpeDl.h"
#include "IxOsal.h"


/*
 * #defines and macros
 */

/**
 * @def IX_NPEDL_IMAGEMGR_SIGNATURE
 *
 * @brief Signature found as 1st word in a microcode image library
 */
#define IX_NPEDL_IMAGEMGR_SIGNATURE      0xDEADBEEF

/**
 * @def IX_NPEDL_IMAGEMGR_END_OF_HEADER
 *
 * @brief Marks end of header in a microcode image library
 */
#define IX_NPEDL_IMAGEMGR_END_OF_HEADER  0xFFFFFFFF

/**
 * @def IX_NPEDL_IMAGEID_NPEID_OFFSET
 *
 * @brief Offset from LSB of NPE ID field in Image ID
 */
#define IX_NPEDL_IMAGEID_NPEID_OFFSET      24

/**
 * @def IX_NPEDL_IMAGEID_DEVICEID_OFFSET
 *
 * @brief Offset from LSB of Device ID field in Image ID
 */
#define IX_NPEDL_IMAGEID_DEVICEID_OFFSET   28

/**
 * @def IX_NPEDL_IMAGEID_FUNCTIONID_OFFSET
 *
 * @brief Offset from LSB of Functionality ID field in Image ID
 */
#define IX_NPEDL_IMAGEID_FUNCTIONID_OFFSET 16

/**
 * @def IX_NPEDL_IMAGEID_MAJOR_OFFSET
 *
 * @brief Offset from LSB of Major revision field in Image ID
 */
#define IX_NPEDL_IMAGEID_MAJOR_OFFSET      8

/**
 * @def IX_NPEDL_IMAGEID_MINOR_OFFSET
 *
 * @brief Offset from LSB of Minor revision field in Image ID
 */
#define IX_NPEDL_IMAGEID_MINOR_OFFSET      0

/**
 * @def IX_NPEDL_NPEID_FROM_IMAGEID_GET
 *
 * @brief Macro to extract NPE ID field from Image ID
 */
#define IX_NPEDL_NPEID_FROM_IMAGEID_GET(imageId) \
    (((imageId) >> IX_NPEDL_IMAGEID_NPEID_OFFSET) & \
     IX_NPEDL_NPEIMAGE_NPEID_MASK)

/**
 * @def IX_NPEDL_DEVICEID_FROM_IMAGEID_GET
 *
 * @brief Macro to extract NPE ID field from Image ID
 */
#define IX_NPEDL_DEVICEID_FROM_IMAGEID_GET(imageId) \
    (((imageId) >> IX_NPEDL_IMAGEID_DEVICEID_OFFSET) & \
     IX_NPEDL_NPEIMAGE_DEVICEID_MASK)

/**
 * @def IX_NPEDL_FUNCTIONID_FROM_IMAGEID_GET
 *
 * @brief Macro to extract Functionality ID field from Image ID
 */
#define IX_NPEDL_FUNCTIONID_FROM_IMAGEID_GET(imageId) \
    (((imageId) >> IX_NPEDL_IMAGEID_FUNCTIONID_OFFSET) & \
     IX_NPEDL_NPEIMAGE_FIELD_MASK)

/**
 * @def IX_NPEDL_MAJOR_FROM_IMAGEID_GET
 *
 * @brief Macro to extract Major revision field from Image ID
 */
#define IX_NPEDL_MAJOR_FROM_IMAGEID_GET(imageId) \
    (((imageId) >> IX_NPEDL_IMAGEID_MAJOR_OFFSET) & \
     IX_NPEDL_NPEIMAGE_FIELD_MASK)

/**
 * @def IX_NPEDL_MINOR_FROM_IMAGEID_GET
 *
 * @brief Macro to extract Minor revision field from Image ID
 */
#define IX_NPEDL_MINOR_FROM_IMAGEID_GET(imageId) \
    (((imageId) >> IX_NPEDL_IMAGEID_MINOR_OFFSET) & \
     IX_NPEDL_NPEIMAGE_FIELD_MASK)

/*
 * Prototypes for interface functions
 */

/**
 * @fn void ixNpeDlImageMgrStatsShow (void)
 *
 * @brief This function will display the statistics of the IxNpeDl ImageMgr
 *        module
 *
 * @return none
 */
void
ixNpeDlImageMgrStatsShow (void);


/**
 * @fn void ixNpeDlImageMgrStatsReset (void)
 *
 * @brief This function will reset the statistics of the IxNpeDl ImageMgr
 *        module
 *
 * @return none
 */
void
ixNpeDlImageMgrStatsReset (void);


/**
 * @fn IX_STATUS ixNpeDlImageMgrImageGet (UINT32 *imageLibrary,
                                          UINT32 imageId,
                                          UINT32 **imagePtr,
                                          UINT32 *imageSize)
 * 
 * @brief Finds a image block in the NPE microcode image library. 
 *
 * @param UINT32*  [in]  imageLibrary - the image library to use
 * @param UINT32   [in]  imageId      - the id of the image to locate
 * @param UINT32** [out] imagePtr     - pointer to the image in memory
 * @param UINT32*  [out] imageSize    - size (in 32-bit words) of image
 * 
 * This function examines the header of the specified microcode image library
 * for the location and size of the specified image.  It returns a pointer to
 * the image in the <i>imagePtr</i> parameter.
 * If no image library is specified (imageLibrary == NULL), then the default
 * built-in image library will be used.
 * 
 * @pre
 *
 * @post
 *
 * @return 
 *      - IX_SUCCESS if the operation was successful
 *      - IX_FAIL otherwise
 */ 
IX_STATUS
ixNpeDlImageMgrImageFind (UINT32 *imageLibrary,
                          UINT32 imageId,
			  UINT32 **imagePtr,
			  UINT32 *imageSize);


#endif /* IXNPEDLIMAGEMGR_P_H */

/**
 * @} defgroup IxNpeDlImageMgr_p
 */
