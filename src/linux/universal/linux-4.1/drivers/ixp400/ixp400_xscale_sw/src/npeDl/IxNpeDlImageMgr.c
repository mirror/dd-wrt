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

