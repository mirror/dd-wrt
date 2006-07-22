/**
 * @file IxNpeDlImageMgr.c
 *
 * @author Intel Corporation
 * @date 09 January 2002
 *
 * @brief This file contains the implementation of the private API for the 
 *        IXP400 NPE Downloader ImageMgr module
 *
 * 
 * @par
 * IXP400 SW Release version  2.1
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
*/


/*
 * Put the system defined include files required.
 */
#include "IxOsal.h"

/*
 * Put the user defined include files required.
 */
#include "IxNpeDlImageMgr_p.h"
#include "IxNpeDlMacros_p.h"

/*
 * define the flag which toggles the firmare inclusion
 */
#define IX_NPE_MICROCODE_FIRMWARE_INCLUDED 1
#include "IxNpeMicrocode.h"

/*
 * Indicates the start of an NPE Image, in new NPE Image Library format.
 * 2 consecutive occurances indicates the end of the NPE Image Library
 */
#define NPE_IMAGE_MARKER 0xfeedf00d

/*
 * Typedefs whose scope is limited to this file.
 */

/*
 * NPE Image Header definition, used in new NPE Image Library format
 */
typedef struct
{
    UINT32 marker;
    UINT32 id;
    UINT32 size;
} IxNpeDlImageMgrImageHeader;

/* module statistics counters */
typedef struct
{
    UINT32 invalidSignature;
    UINT32 imageIdListOverflow;
    UINT32 imageIdNotFound;
} IxNpeDlImageMgrStats;


/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */
static IxNpeDlImageMgrStats ixNpeDlImageMgrStats;

/* default image */
#ifndef IX_NPEDL_READ_MICROCODE_FROM_FILE
static UINT32 *IxNpeMicroCodeImageLibrary = (UINT32 *)IxNpeMicrocode_array;
#endif


/*
 * Function definition: ixNpeDlImageMgrStatsShow
 */
void
ixNpeDlImageMgrStatsShow (void)
{
    ixOsalLog (IX_OSAL_LOG_LVL_USER,
               IX_OSAL_LOG_DEV_STDOUT,
               "\nixNpeDlImageMgrStatsShow:\n"
               "\tInvalid Image Signatures: %u\n"
               "\tImage Id List capacity too small: %u\n"
               "\tImage Id not found: %u\n\n",
               ixNpeDlImageMgrStats.invalidSignature,
               ixNpeDlImageMgrStats.imageIdListOverflow,
               ixNpeDlImageMgrStats.imageIdNotFound,
               0,0,0);
}


/*
 * Function definition: ixNpeDlImageMgrStatsReset
 */
void
ixNpeDlImageMgrStatsReset (void)
{
    ixNpeDlImageMgrStats.invalidSignature = 0;
    ixNpeDlImageMgrStats.imageIdListOverflow = 0;
    ixNpeDlImageMgrStats.imageIdNotFound = 0;
}


/*
 * Function definition: ixNpeDlImageMgrImageFind
 */
#ifdef KEEP_UNUSED 
IX_STATUS
ixNpeDlImageMgrImageFind (
    UINT32 *imageLibrary,
    UINT32 imageId,
    UINT32 **imagePtr,
    UINT32 *imageSize)
{
    IxNpeDlImageMgrImageHeader *image;
    UINT32 offset = 0;

    /* If user didn't specify a library to use, use the default
     * one from IxNpeMicrocode.h
     */
    if (imageLibrary == NULL)
    {
#ifdef IX_NPEDL_READ_MICROCODE_FROM_FILE
	if (ixNpeMicrocode_binaryArray == NULL)
        {
	    printk (KERN_ERR "ixp400.o:  ERROR, no Microcode found in memory\n");
	    return IX_FAIL;
	}
	else
	{
	    imageLibrary = ixNpeMicrocode_binaryArray;
	}
#else
	imageLibrary = IxNpeMicroCodeImageLibrary;
#endif /* IX_NPEDL_READ_MICROCODE_FROM_FILE */
    }

    while (*(imageLibrary+offset) == NPE_IMAGE_MARKER)
    {
        image = (IxNpeDlImageMgrImageHeader *)(imageLibrary+offset);
        offset += sizeof(IxNpeDlImageMgrImageHeader)/sizeof(UINT32);
        
        if (image->id == imageId)
        {
            *imagePtr = imageLibrary + offset;
            *imageSize = image->size;
            return IX_SUCCESS;
        }
        /* 2 consecutive NPE_IMAGE_MARKER's indicates end of library */
        else if (image->id == NPE_IMAGE_MARKER)
        {
	    IX_NPEDL_ERROR_REPORT ("ixNpeDlImageMgrImageFind: "
				   "imageId not found in image library header\n");
	    ixNpeDlImageMgrStats.imageIdNotFound++;
            /* reached end of library, image not found */
            return IX_FAIL;
        }
        offset += image->size;
    }

    /* If we get here, our image library may be corrupted */
    IX_NPEDL_ERROR_REPORT ("ixNpeDlImageMgrImageFind: "
                           "image library format may be invalid or corrupted\n");
    return IX_FAIL;
}

#endif
