/*===========================================================================
FILE: 
   GobiMBNMgmt.h

DESCRIPTION:
   QUALCOMM Gobi MBN management functions for Gobi 3000

PUBLIC CLASSES AND FUNCTIONS:
   sImageInfo
   GetImageStore
   GetImageInfo
   GetImagesInfo
   GetImageBootCompatibility
   MapVersionInfo
   GetImageByUniqueID 

Copyright (c) 2011, Code Aurora Forum. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Code Aurora Forum nor
      the names of its contributors may be used to endorse or promote
      products derived from this software without specific prior written
      permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
==========================================================================*/

/*=========================================================================*/
// Pragmas
/*=========================================================================*/
#pragma once

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "GobiImageDefinitions.h"
#include "GobiError.h"

#include <vector>

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// Maximum length for unique image ID
const ULONG MBN_UNIQUE_ID_LEN = 16;

/*=========================================================================*/
// Struct sImageInfo
//    Storage structure for Image information
/*=========================================================================*/
struct sImageInfo
{
   public:
      // Default constructor
      sImageInfo()
         :  mImageType( eGOBI_MBN_TYPE_ENUM_BEGIN ),
            mVersionID( ULONG_MAX ),
            mVersion( "" )
      {
         memset( (LPVOID)&mImageID[0], 0, MBN_UNIQUE_ID_LEN );
      };

      // Is this object valid?
      bool IsValid() const
      {
         return ( (mImageType != eGOBI_MBN_TYPE_ENUM_BEGIN)
             &&   (mVersionID != ULONG_MAX)
             &&   (mVersion.size() > 0) );
      };

      /* Image type */
      eGobiMBNType mImageType;

      /* Unique image ID */
      BYTE mImageID[MBN_UNIQUE_ID_LEN];

      /* Version ID */
      ULONG mVersionID;

      /* Version string */
      std::string mVersion;
};

/*=========================================================================*/
// Public Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   GetImageStore (Public Method)

DESCRIPTION:
   Return the image store folder, i.e., the folder containing one or more 
   carrier-specific image subfolders

RETURN VALUE:
   std::string - Image Store
===========================================================================*/
std::string GetImageStore();

/*===========================================================================
METHOD:
   GetImageInfo (Public Method)

DESCRIPTION:
   Get the image information for the image specified by the given fully 
   qualified path

PARAMETERS:
   pFilePath   [ I ] - Fully qualified path to image file
   pImageType  [ O ] - Image type
   pImageID    [ O ] - Unique image ID
   pVersionID  [ O ] - Version ID
   versionSize [ I ] - The maximum number of characters including the NULL 
                       terminator that can be copied to the version array
   pVersion    [ O ] - NULL-terminated string representing the version

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError GetImageInfo(
   LPCSTR                     pFilePath,
   BYTE *                     pImageType,
   BYTE *                     pImageID,
   ULONG *                    pVersionID,
   USHORT                     versionSize,
   CHAR *                     pVersion );

/*===========================================================================
METHOD:
   GetImagesInfo (Public Method)

DESCRIPTION:
   Return the info for the images located at the given path
  
PARAMETERS:
   path        [ I ] - Fully qualified path

RETURN VALUE:
   std:vector <sImageInfo> - Vector of image information
===========================================================================*/
std::vector <sImageInfo> GetImagesInfo( const std::string & path );

/*===========================================================================
METHOD:
   GetImageBootCompatibility (Public Method)

DESCRIPTION:
   Get the image boot compatibility for the image specified by the given 
   fully qualified path

PARAMETERS:
   pFilePath      [ I ] - Fully qualified path to image file
   pMajorVersion  [ O ] - Major version of compatible boot downloader
   pMinorVersion  [ O ] - Minor version of compatible boot downloader

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError GetImageBootCompatibility(
   LPCSTR                     pFilePath,
   ULONG *                    pMajorVersion,
   ULONG *                    pMinorVersion );

/*===========================================================================
METHOD:
   MapVersionInfo (Public Method)

DESCRIPTION:
   Map the specified version string to image capabilities

PARAMETERS:
   versionID      [ I ] - Version ID
   imageType      [ I ] - Image type
   pVersion       [ I ] - Version string for image
   pTechnology    [ O ] - Technology type
   pCarrier       [ O ] - Carrier type
   pRegion        [ O ] - Region type
   pGPSCapability [ O ] - GPS capability

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError MapVersionInfo(
   ULONG                      versionID,
   BYTE                       imageType,
   LPCSTR                     pVersion,
   ULONG *                    pTechnology,
   ULONG *                    pCarrier,
   ULONG *                    pRegion,
   ULONG *                    pGPSCapability );

/*===========================================================================
METHOD:
   GetImageByUniqueID (Public Method)

DESCRIPTION:
   Return the fully qualified path to an image specified by unique ID
  
PARAMETERS:
   pImageID    [ I ] - Unique image ID

RETURN VALUE:
   std::string - Fully qualified path to matching image
===========================================================================*/
std::string GetImageByUniqueID( BYTE * pImageID );


