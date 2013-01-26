/*===========================================================================
FILE: 
   GobiMBNMgmt.cpp

DESCRIPTION:
   QUALCOMM Gobi MBN management functions for Gobi 3000

PUBLIC CLASSES AND FUNCTIONS:
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

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "StdAfx.h"
#include "GobiMBNMgmt.h"
#include "GobiError.h"

#include "CoreUtilities.h"
#include "MemoryMappedFile.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// Magic values for MBN (AMSS/UQCN) images
const ULONG MBN_LOCK_MAGIC      = 0x809b1d80;
const ULONG MBN_BOOT_MAGIC      = 0xFEDC1234;
const ULONG MBN_BUILD_MAGIC     = 0xFEDC1235;
const ULONG UQCN_INFO_MAGIC     = 0xFEDC1236;
const ULONG MBN_HASH_MAGIC      = 0xFEDC1237;
const ULONG MBN_LOCK_AUTH_MAGIC = 0xFEDC1238;

// Maximum length for an UQCN build info string (including NULL)
const ULONG MBN_BUILD_ID_LEN = 32;

//---------------------------------------------------------------------------
// Pragmas (pack structs)
//---------------------------------------------------------------------------
#pragma pack( push, 1 )

/*=========================================================================*/
// Struct sMBNBootRecord
//    Struct to represent the MBN boot flash record
/*=========================================================================*/
struct sMBNBootRecord
{
   public:
      ULONG mMagic;  // MBN_BOOT_MAGIC
      WORD mMajorID; // Boot flash major version
      WORD mMinorID; // Boot flash minor version
};

/*=========================================================================*/
// Struct sMBNBuildRecord
//    Struct to represent the build ID record
/*=========================================================================*/
struct sMBNBuildIDRecord
{
   public:
      ULONG mMagic;                    // MBN_BUILD_MAGIC
      CHAR mBuildID[MBN_BUILD_ID_LEN]; // Build ID string
};

/*=========================================================================*/
// Struct sUQCNVersionID
//    Struct to represent the UQCN version ID
/*=========================================================================*/
struct sUQCNVersionID
{
   public:
      ULONG mMinorID       : 7;
      ULONG mXTRADisabled  : 1;
      ULONG mGPSDisabled   : 1;
      ULONG mReserved      : 7;
      ULONG mMajorID       : 8;
      ULONG mSystem        : 2;
      ULONG mCompatibility : 6;
};

/*=========================================================================*/
// Struct sUQCNInfoRecord
//    Struct to represent the UQCN information record
/*=========================================================================*/
struct sUQCNInfoRecord
{
   public:
      ULONG mMagic;                 // UQCN_INFO_MAGIC
      sUQCNVersionID mVersionID;    // Version ID
      CHAR mInfo[MBN_BUILD_ID_LEN]; // Build info string
};

/*=========================================================================*/
// Struct sMBNHashRecord
//    Struct to represent the signature hash record
/*=========================================================================*/
struct sMBNHashRecord
{
   public:
      ULONG mMagic;                       // MBN_HASH_MAGIC
      BYTE mUniqueID[MBN_UNIQUE_ID_LEN];  // Build ID string
};

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma pack( pop )

/*=========================================================================*/
// Free Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   ReverseBinaryDataSearch (Free Method)

DESCRIPTION:
   Search a data buffer for the first occurence of a specified
   sequence of data (starting from the end of the buffer)

PARAMETERS:
   pBuffer     [ I ] - Buffer being search
   bufferLen   [ I ] - Length of above buffer
   pData       [ I ] - Data to search for
   dataLen     [ I ] - Length of above buffer

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
const BYTE * ReverseBinaryDataSearch( 
   const BYTE *               pBuffer,
   ULONG                      bufferLen,
   const BYTE *               pData, 
   ULONG                      dataLen ) 
{ 
   // Handle empty cases
   if (pBuffer == 0 || bufferLen == 0 || pData ==0 || dataLen == 0)
   {
      return 0;
   }

   // Handle impossible case
   if (dataLen > bufferLen)
   {
      return 0;
   }

   const BYTE * pTmp = pBuffer + (bufferLen - dataLen);
   while (pTmp > pBuffer)
   {
      int res = ::memcmp( (const void *)pTmp, 
                          (const void *)pData, 
                          (size_t)dataLen );

      if (res == 0)
      {
         return pTmp;
      }

      pTmp--;
   }

   return 0;
}

/*===========================================================================
METHOD:
   ParseUQCNVersion (Free Method)

DESCRIPTION:
   Parse UQCN version ID to image information

PARAMETERS:
   uqcnID         [ I ] - UQCN ID
   pTechnology    [ O ] - Technology (0xFFFFFFFF if unknown)
   pCarrier       [ O ] - Carrier (0xFFFFFFFF if unknown)
   pRegion        [ O ] - Region (0xFFFFFFFF if unknown)
   pGPSCapability [ O ] - GPS capability (0xFFFFFFFF if unknown)

RETURN VALUE:
   bool
===========================================================================*/
bool ParseUQCNVersion( 
   ULONG                      uqcnID, 
   ULONG *                    pTechnology,
   ULONG *                    pCarrier,
   ULONG *                    pRegion,
   ULONG *                    pGPSCapability )
{
   // Assume failure
   bool bRC = false;
   *pTechnology    = ULONG_MAX;
   *pCarrier       = ULONG_MAX;
   *pRegion        = ULONG_MAX;
   *pGPSCapability = ULONG_MAX;

   sUQCNVersionID * pID = (sUQCNVersionID *)&uqcnID;
   if (pID->mSystem == 2)
   {
      // Successs is returned when the technology is valid  
      *pTechnology = (ULONG)eGOBI_IMG_TECH_UMTS;
      bRC = true;
   }
   else if (pID->mSystem == 1)
   {
      // Successs is returned when the technology is valid  
      *pTechnology = (ULONG)eGOBI_IMG_TECH_CDMA;
      bRC = true;
   }

   // Valid technology?
   if (bRC == false)
   {
      return bRC;
   }
      
   switch (pID->mMajorID)
   {
      case 0x00:
         *pCarrier = (ULONG)eGOBI_IMG_CAR_FACTORY;
         *pRegion  = (ULONG)eGOBI_IMG_REG_NA;
         break;

      case 0x01:
         *pCarrier = (ULONG)eGOBI_IMG_CAR_VERIZON;
         *pRegion  = (ULONG)eGOBI_IMG_REG_NA;
         break;
         
      case 0x02:
         *pCarrier = (ULONG)eGOBI_IMG_CAR_SPRINT;
         *pRegion  = (ULONG)eGOBI_IMG_REG_NA;
         break;         
         
      case 0x03: 
         *pCarrier = (ULONG)eGOBI_IMG_CAR_ATT;
         *pRegion  = (ULONG)eGOBI_IMG_REG_NA;
         break;          
         
      case 0x04:
         *pCarrier = (ULONG)eGOBI_IMG_CAR_VODAFONE;
         *pRegion  = (ULONG)eGOBI_IMG_REG_EU;
         break;           
         
      case 0x05:
         *pCarrier = (ULONG)eGOBI_IMG_CAR_TMOBILE;
         *pRegion  = (ULONG)eGOBI_IMG_REG_EU;
         break;                          

      case 0x09:
         *pCarrier = (ULONG)eGOBI_IMG_CAR_GENERIC;
         *pRegion  = (ULONG)eGOBI_IMG_REG_GLOBAL;
         break;     

      case 0x0B:
         *pCarrier = (ULONG)eGOBI_IMG_CAR_ORANGE;
         *pRegion  = (ULONG)eGOBI_IMG_REG_EU;
         break;     

      case 0x0C: 
         *pCarrier = (ULONG)eGOBI_IMG_CAR_TELEFONICA;
         *pRegion  = (ULONG)eGOBI_IMG_REG_EU;
         break;     

      case 0x0D: 
         *pCarrier = (ULONG)eGOBI_IMG_CAR_NTT_DOCOMO;
         *pRegion  = (ULONG)eGOBI_IMG_REG_ASIA;
         break;     

      case 0x0E:
         *pCarrier = (ULONG)eGOBI_IMG_CAR_TELCOM_ITALIA;
         *pRegion  = (ULONG)eGOBI_IMG_REG_EU;
         break;     

      case 0x12:
         *pCarrier = (ULONG)eGOBI_IMG_CAR_TELCOM_NZ;
         *pRegion  = (ULONG)eGOBI_IMG_REG_AUS;
         break;     

      case 0x13:
         *pCarrier = (ULONG)eGOBI_IMG_CAR_CHINA_TELECOM;
         *pRegion  = (ULONG)eGOBI_IMG_REG_ASIA;
         break;      

      case 0x14:
         *pCarrier = (ULONG)eGOBI_IMG_CAR_OMH;
         *pRegion  = (ULONG)eGOBI_IMG_REG_GLOBAL;
         break;     

      case 0x16:
         *pCarrier = (ULONG)eGOBI_IMG_CAR_AMX_TELCEL;
         *pRegion  = (ULONG)eGOBI_IMG_REG_LA;
         break;     

      case 0x17:
         *pCarrier = (ULONG)eGOBI_IMG_CAR_NORF;
         *pRegion  = (ULONG)eGOBI_IMG_REG_GLOBAL;
         break;

      case 0x18:
         *pCarrier = (ULONG)eGOBI_IMG_CAR_FACTORY;
         *pRegion  = (ULONG)eGOBI_IMG_REG_GLOBAL;
         break;  

      case 0x19:
         *pCarrier = (ULONG)eGOBI_IMG_CAR_BRASIL_VIVO;
         *pRegion  = (ULONG)eGOBI_IMG_REG_LA;
         break;  
   }

   // Set GPS capability
   if (pID->mGPSDisabled == 1)
   {
      *pGPSCapability = (ULONG)eGOBI_IMG_GPS_NONE;
   }
   else if (*pCarrier == (ULONG)eGOBI_IMG_CAR_NORF)
   {
      // No RF with GPS results in stand-alone GPS support only
      *pGPSCapability = (ULONG)eGOBI_IMG_GPS_STAND_ALONE;
   }
   else
   {
      if (pID->mXTRADisabled == 1)
      {
         *pGPSCapability = (ULONG)eGOBI_IMG_GPS_NO_XTRA;
      }
      else
      {
         *pGPSCapability = (ULONG)eGOBI_IMG_GPS_ASSISTED;
      }
   }

   return bRC;
}

/*===========================================================================
METHOD:
   ParseAMSSVersion (Free Method)

DESCRIPTION:
   Parse UQCN version ID to image information

PARAMETERS:
   pVersion       [ I ] - Version string
   pTechnology    [ O ] - Technology (0xFFFFFFFF if unknown)
   pCarrier       [ O ] - Carrier (0xFFFFFFFF if unknown)
   pRegion        [ O ] - Region (0xFFFFFFFF if unknown)
   pGPSCapability [ O ] - GPS capability (0xFFFFFFFF if unknown)

RETURN VALUE:
   bool
===========================================================================*/
bool ParseAMSSVersion( 
   LPCSTR                     pVersion,
   ULONG *                    pTechnology,
   ULONG *                    pCarrier,
   ULONG *                    pRegion,
   ULONG *                    pGPSCapability )
{
   // Assume failure
   bool bRC = false;
   *pTechnology    = ULONG_MAX;
   *pCarrier       = ULONG_MAX;
   *pRegion        = ULONG_MAX;
   *pGPSCapability = ULONG_MAX;

   // Validate arguments
   if (pVersion == 0 || pVersion[0] == 0)
   {
      return bRC;
   }

   std::string tmpVer = pVersion;
   
   // std::string version of MakeUpper()
   transform( tmpVer.begin(), tmpVer.end(), tmpVer.begin(), toupper );

   if ( (tmpVer.find( "STAUFH" ) != std::string::npos)
   ||   (tmpVer.find( "STSUFH" ) != std::string::npos) ) 
   {
      *pTechnology = (ULONG)eGOBI_IMG_TECH_CDMA;
      *pCarrier    = (ULONG)eGOBI_IMG_CAR_FACTORY;
      *pRegion     = (ULONG)eGOBI_IMG_REG_GLOBAL;

      bRC = true;
      return bRC;
   }
   else if ( (tmpVer.find( "STAUVH" ) != std::string::npos)
        ||   (tmpVer.find( "STSUVH" ) != std::string::npos) ) 
   {
      *pTechnology = (ULONG)eGOBI_IMG_TECH_CDMA;
      *pCarrier    = (ULONG)eGOBI_IMG_CAR_VERIZON;
      *pRegion     = (ULONG)eGOBI_IMG_REG_NA;

      bRC = true;
      return bRC;
   }
   else if ( (tmpVer.find( "STAUSH" ) != std::string::npos)
        ||   (tmpVer.find( "STSUSH" ) != std::string::npos) ) 
   {
      *pTechnology = (ULONG)eGOBI_IMG_TECH_CDMA;
      *pCarrier    = (ULONG)eGOBI_IMG_CAR_SPRINT;
      *pRegion     = (ULONG)eGOBI_IMG_REG_NA;

      bRC = true;
      return bRC;
   }
   else if (tmpVer.find( "STSUCH" ) != std::string::npos)
   {
      *pTechnology = (ULONG)eGOBI_IMG_TECH_CDMA;
      *pCarrier    = (ULONG)eGOBI_IMG_CAR_CHINA_TELECOM;
      *pRegion     = (ULONG)eGOBI_IMG_REG_ASIA;

      bRC = true;
      return bRC;
   }
   else if (tmpVer.find( "STSUOH" ) != std::string::npos)
   {
      *pTechnology = (ULONG)eGOBI_IMG_TECH_CDMA;
      *pCarrier    = (ULONG)eGOBI_IMG_CAR_OMH;
      *pRegion     = (ULONG)eGOBI_IMG_REG_GLOBAL;

      bRC = true;
      return bRC;
   }
   else if ( (tmpVer.find( "STAUXN" ) != std::string::npos)
        ||   (tmpVer.find( "STSUXN" ) != std::string::npos) ) 
   {
      *pTechnology = (ULONG)eGOBI_IMG_TECH_UMTS;
      *pCarrier    = (ULONG)eGOBI_IMG_CAR_NORF;
      *pRegion     = (ULONG)eGOBI_IMG_REG_GLOBAL;

      bRC = true;
      return bRC;
   }
   else if ( (tmpVer.find( "STAUFN" ) != std::string::npos)
        ||   (tmpVer.find( "STSUFN" ) != std::string::npos) ) 
   {
      *pTechnology = (ULONG)eGOBI_IMG_TECH_UMTS;
      *pCarrier    = (ULONG)eGOBI_IMG_CAR_FACTORY;
      *pRegion     = (ULONG)eGOBI_IMG_REG_GLOBAL;

      bRC = true;
      return bRC;
   }
   else if ( (tmpVer.find( "STAUGN" ) != std::string::npos)
        ||   (tmpVer.find( "STSUGN" ) != std::string::npos) ) 
   {
      *pTechnology = (ULONG)eGOBI_IMG_TECH_UMTS;
      *pCarrier    = (ULONG)eGOBI_IMG_CAR_GENERIC;
      *pRegion     = (ULONG)eGOBI_IMG_REG_GLOBAL;

      bRC = true;
      return bRC;
   }

   return bRC;
}

/*===========================================================================
METHOD:
   GetImageStore (Public Method)

DESCRIPTION:
   Get the image store folder, i.e., the folder containing one or more 
   carrier-specific image subfolders

RETURN VALUE:
   std::string - Image Store
===========================================================================*/
std::string GetImageStore()
{
   std::string imageStore = GetProgramPath();
   imageStore += "Images/3000/Generic";

   return imageStore;
}

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
   CHAR *                     pVersion )
{
   // Validate arguments
   if ( (pFilePath == 0)
   ||   (pFilePath[0] == 0)
   ||   (pImageType == 0)
   ||   (pImageID == 0) 
   ||   (pVersionID == 0) 
   ||   (versionSize == 0) 
   ||   (pVersion == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Open up MBN file
   cMemoryMappedFile mbnFile( pFilePath );
   const BYTE * pMBNData = (const BYTE *)mbnFile.GetContents();
   ULONG dataSz = mbnFile.GetSize();

   // MBN file (sort of) valid?
   if (pMBNData == 0)
   {
      return eGOBI_ERR_FILE_OPEN;
   }
   
   if (dataSz <= 256)
   {
      return eGOBI_ERR_INVALID_FILE;
   }

   // Skip to the end
   pMBNData += (dataSz - 256);

   // Search for the UQCN specific info
   const BYTE * pTmp = 0;
   pTmp = ReverseBinaryDataSearch( pMBNData,
                                   256,
                                   (const BYTE *)&UQCN_INFO_MAGIC,
                                   (ULONG)sizeof( UQCN_INFO_MAGIC ) );

   if (pTmp != 0)
   {
      const sUQCNInfoRecord * pRec = (const sUQCNInfoRecord *)pTmp;
      *pVersionID = *(ULONG *)&pRec->mVersionID;
      *pImageType = 1;
   }
   else
   {
      // Since we did not find UQCN info, presume this is an AMSS file
      pTmp = ReverseBinaryDataSearch( pMBNData,
                                      256,
                                      (const BYTE *)&MBN_BOOT_MAGIC,
                                      (ULONG)sizeof( MBN_BOOT_MAGIC ) );

      if (pTmp == 0)
      {
         return eGOBI_ERR_INVALID_FILE;
      }

      const sMBNBootRecord * pRec = (const sMBNBootRecord *)pTmp;
      *pVersionID = pRec->mMinorID;
      *pImageType = 0;
   }

   // Search for the unique ID
   pTmp = ReverseBinaryDataSearch( pMBNData,
                                   256,
                                   (const BYTE *)&MBN_HASH_MAGIC,
                                   (ULONG)sizeof( MBN_HASH_MAGIC ) );

   if (pTmp == 0)
   {
      return eGOBI_ERR_INVALID_FILE;
   }

   // Copy the unique ID
   const sMBNHashRecord * pHash = (const sMBNHashRecord *)pTmp;
   memcpy( (LPVOID)pImageID,
           (LPCVOID)&pHash->mUniqueID[0],
           (SIZE_T)MBN_UNIQUE_ID_LEN );


   // Search for the build ID
   pTmp = ReverseBinaryDataSearch( pMBNData,
                                   256,
                                   (const BYTE *)&MBN_BUILD_MAGIC,
                                   (ULONG)sizeof( MBN_BUILD_MAGIC ) );

   if (pTmp == 0)
   {
      return eGOBI_ERR_INVALID_FILE;
   }

   memset( (PVOID)&pVersion[0], 0, (SIZE_T)versionSize );

   // Copy the MBN_BUILD_MAGIC ID
   const sMBNBuildIDRecord * pRec = (const sMBNBuildIDRecord *)pTmp;
   for (ULONG t = 0; t < MBN_BUILD_ID_LEN; t++)
   {
      if (t >= versionSize)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      pVersion[t] = pRec->mBuildID[t];
      if (pRec->mBuildID[t] == 0)
      {
         break;
      }
   }

   return eGOBI_ERR_NONE;
}

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
std::vector <sImageInfo> GetImagesInfo( const std::string & path )
{
   // Validate arguments
   std::vector <sImageInfo> retVec;
   if (path.size() <= 0)
   {
      return retVec;
   }

   // Search all MBN files in the specified folder
   std::string folderSearch = path;

   int folderLen = folderSearch.size();
   if (folderSearch[folderLen - 1] != '/')
   {
      folderSearch += '/';
   }

   std::vector <std::string> files;
   DepthSearch( folderSearch,
                0,
                ".mbn",
                files );

   int fileNum = files.size();
   for (int i = 0; i < fileNum; i++)
   {
      std::string mbnName = files[i];

      BYTE imageType = UCHAR_MAX;
      BYTE imageID[16] = { 0 };
      ULONG versionID = ULONG_MAX;
      USHORT versionSz = MAX_PATH * 2 + 1;
      CHAR versionStr[MAX_PATH * 2 + 1] = { 0 };
      eGobiError rc = ::GetImageInfo( mbnName.c_str(),
                                      &imageType,
                                      &imageID[0],
                                      &versionID,
                                      versionSz,
                                      &versionStr[0] );

      if (rc == eGOBI_ERR_NONE)
      {
         sImageInfo ii;
         ii.mImageType = (eGobiMBNType)imageType;
         ii.mVersionID = versionID;
         ii.mVersion = (LPCSTR)&versionStr[0];
         memcpy( (LPVOID)&ii.mImageID[0], (LPCVOID)&imageID[0], 16 );

         retVec.push_back( ii );
      }
   }

   return retVec;
}

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
   ULONG *                    pMinorVersion )
{
   // Validate arguments
   if ( (pFilePath == 0)
   ||   (pFilePath[0] == 0)
   ||   (pMajorVersion == 0)
   ||   (pMinorVersion == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Open up MBN file
   cMemoryMappedFile mbnFile( pFilePath );
   const BYTE * pMBNData = (const BYTE *)mbnFile.GetContents();
   ULONG dataSz = mbnFile.GetSize();

   // MBN file (sort of) valid?
   if (pMBNData == 0)
   {
      return eGOBI_ERR_FILE_OPEN;
   }
   
   if (dataSz <= 256)
   {
      return eGOBI_ERR_INVALID_FILE;
   }

   // Skip to the end
   pMBNData += (dataSz - 256);

   const BYTE * pTmp = 0;
   pTmp = ReverseBinaryDataSearch( pMBNData,
                                   256,
                                   (const BYTE *)&MBN_BOOT_MAGIC,
                                   (ULONG)sizeof( MBN_BOOT_MAGIC ) );

   if (pTmp == 0)
   {
      return eGOBI_ERR_INVALID_FILE;
   }

   const sMBNBootRecord * pRec = (const sMBNBootRecord *)pTmp;
   *pMajorVersion = pRec->mMajorID;
   *pMinorVersion = pRec->mMinorID;

   return eGOBI_ERR_NONE;
}

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
   ULONG *                    pGPSCapability )
{
   if ((eGobiMBNType)imageType == eGOBI_MBN_TYPE_MODEM)
   {
      // AMSS (modem)
      bool bOK = ParseAMSSVersion( pVersion,
                                   pTechnology,
                                   pCarrier,
                                   pRegion,
                                   pGPSCapability );

      if (bOK == false)
      {
         return eGOBI_ERR_INVALID_ARG;
      }
   }
   else if ((eGobiMBNType)imageType == eGOBI_MBN_TYPE_PRI)
   {
      // UQCN (PRI)
      bool bOK = ParseUQCNVersion( versionID,
                                   pTechnology,
                                   pCarrier,
                                   pRegion,
                                   pGPSCapability );

      if (bOK == false)
      {
         return eGOBI_ERR_INVALID_ARG;
      }
   }

   return eGOBI_ERR_NONE;
}

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
std::string GetImageByUniqueID( BYTE * pImageID )
{
   // Validate arguments
   std::string retStr = "";
   if (pImageID == 0)
   {
      return retStr;
   }

   // Enumerate all folders of the image store
   std::vector <std::string> folders;
   std::string imageStore = ::GetImageStore();
   EnumerateFolders( imageStore, folders );

   // Did we find any folders?
   ULONG foldersSz = (ULONG)folders.size();
   if (foldersSz == 0)
   {
      return retStr;
   }

   // Go through each folder searching for a match
   for (ULONG f = 0; f < foldersSz; f++)
   {
      // Search all MBN files in the specified folder
      std::string folderSearch = folders[f];

      int folderLen = folderSearch.size();
      if (folderSearch[folderLen - 1] != '/')
      {
         folderSearch += '/';
      }

      std::vector <std::string> files;
      DepthSearch( folderSearch,
                   0,
                   ".mbn",
                   files );

      int fileNum = files.size();
      for (int i = 0; i < fileNum; i++)
      {
         std::string mbnName = files[i];

         BYTE imageType = UCHAR_MAX;
         BYTE imageID[16] = { 0 };
         ULONG versionID = ULONG_MAX;
         USHORT versionSz = MAX_PATH * 2 + 1;
         CHAR versionStr[MAX_PATH * 2 + 1] = { 0 };
         eGobiError rc = ::GetImageInfo( mbnName.c_str(),
                                         &imageType,
                                         &imageID[0],
                                         &versionID,
                                         versionSz,
                                         &versionStr[0] );

         if (rc == eGOBI_ERR_NONE)
         {
            bool bMatch = true;
            for (ULONG i = 0; i < 16; i++)
            {
               if (imageID[i] != pImageID[i])
               {
                  bMatch = false;
                  break;
               }
            }

            if (bMatch == true)
            {
               retStr = mbnName;
               break;
            }
         }
      }

      if (retStr.size() > 0)
      {
         break;
      }
   }

   return retStr;
}

