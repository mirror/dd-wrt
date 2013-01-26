/*===========================================================================
FILE: 
   GobiQMICoreImg2k.cpp

DESCRIPTION:
   QUALCOMM Gobi QMI Based API Core (Image Management, G2k API)

PUBLIC CLASSES AND FUNCTIONS:
   cGobiQMICore

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
#include "GobiQMICore.h"

#include "QMIBuffers.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// Free Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   GetGenericImage (Free Method)

DESCRIPTION:
   Return a compatible AMSS generic image
  
PARAMETERS:
   uqcnInfo    [ I ] - UQCN image needing a compatible AMSS generic image

RETURN VALUE:
   sImageInfo - Generic image information
===========================================================================*/
sImageInfo GetGenericImage( const sImageInfo & uqcnInfo )
{
   // Validate arguments
   sImageInfo amssInfo;

   // Obtain the technology/carrier of the UQCN
   ULONG uqcnTech;
   ULONG uqcnCarrier;
   ULONG dummy;
   eGobiError rc = ::MapVersionInfo( uqcnInfo.mVersionID,
                                     (BYTE)uqcnInfo.mImageType,
                                     uqcnInfo.mVersion.c_str(),
                                     &uqcnTech,
                                     &uqcnCarrier,
                                     &dummy,
                                     &dummy );

   if (rc != eGOBI_ERR_NONE)
   {
      return amssInfo;
   }

   // Recursively enumerate all folders of the image store
   std::vector <std::string> folders;
   std::string imageStore = ::GetImageStore();
   EnumerateFolders( imageStore, folders );

   // Did we find any folders?
   ULONG foldersSz = (ULONG)folders.size();
   if (foldersSz == 0)
   {
      return amssInfo;
   }

   // Go through each folder searching for a compatible generic AMSS image
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
         rc = ::GetImageInfo( mbnName.c_str(),
                              &imageType,
                              &imageID[0],
                              &versionID,
                              versionSz,
                              &versionStr[0] );

         if (rc == eGOBI_ERR_NONE)
         {
            if ((eGobiMBNType)imageType == eGOBI_MBN_TYPE_MODEM)
            {
               ULONG amssTech;
               ULONG amssCarrier;
               rc = ::MapVersionInfo( versionID,
                                      imageType,
                                      (LPCSTR)&versionStr[0],
                                      &amssTech,
                                      &amssCarrier,
                                      &dummy,
                                      &dummy );

               if (rc == eGOBI_ERR_NONE)
               {
                  if ( (amssTech == uqcnTech) 
                  &&   (amssCarrier == (ULONG)eGOBI_IMG_CAR_GENERIC) )
                  {
                     amssInfo.mImageType = (eGobiMBNType)imageType;
                     amssInfo.mVersionID = versionID;
                     amssInfo.mVersion = (LPCSTR)&versionStr[0];
                     memcpy( (LPVOID)&amssInfo.mImageID[0], 
                             (LPCVOID)&imageID[0], 
                             MBN_UNIQUE_ID_LEN );
                     break;
                  }
               }
            }
         }
      }
   }

   // Success
   return amssInfo;
}

/*=========================================================================*/
// cGobiQMICore Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   GetFirmwareInfo (Public Method)

DESCRIPTION:
   Returns image information obtained from the current device firmware

PARAMETERS:
   pFirmwareID    [ O ] - Firmware ID obtained from the firmware image
   pTechnology    [ O ] - Technology (0xFFFFFFFF if unknown)
   pCarrier       [ O ] - Carrier (0xFFFFFFFF if unknown)
   pRegion        [ O ] - Region (0xFFFFFFFF if unknown)
   pGPSCapability [ O ] - GPS capability (0xFFFFFFFF if unknown)

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetFirmwareInfo( 
   ULONG *                    pFirmwareID,
   ULONG *                    pTechnology,
   ULONG *                    pCarrier,
   ULONG *                    pRegion,
   ULONG *                    pGPSCapability )
{
   // Validate arguments
   if ( (pFirmwareID == 0)
   ||   (pTechnology == 0) 
   ||   (pCarrier == 0)
   ||   (pRegion == 0)
   ||   (pGPSCapability == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Do we have a device node?
   if (mDeviceNode.empty() == true)
   {
      return eGOBI_ERR_NO_CONNECTION;
   }

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_DMS_GET_REV_ID;
   sProtocolBuffer rsp = SendSimple( eQMI_SVC_DMS, msgID );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      return eGOBI_ERR_MALFORMED_RSP;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Prepare TLVs for parsing
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );
   const cCoreDatabase & db = GetDatabase();

   // Parse the TLV we want (by DB key)
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 17 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   std::string uqcnIDString = pf[0].mValueString;
   LONG strLen = uqcnIDString.size();
   if (strLen != 8)
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   std::string idString1 = uqcnIDString.substr( 0, 2 );
   std::string idString2 = uqcnIDString.substr( 2, 2 );
   std::string idString3 = uqcnIDString.substr( 4, 2 );
   std::string idString4 = uqcnIDString.substr( 6, 2 );

   ULONG id1 = 0;
   ULONG id2 = 0;
   ULONG id3 = 0;
   ULONG id4 = 0;
   bool bID1 = StringToULONG( idString1.c_str(), 16, id1 );
   bool bID2 = StringToULONG( idString2.c_str(), 16, id2 );
   bool bID3 = StringToULONG( idString3.c_str(), 16, id3 );
   bool bID4 = StringToULONG( idString4.c_str(), 16, id4 );

   if (bID1 == false || bID2 == false || bID3 == false || bID4 == false)
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   ULONG uqcnID = (id1 << 24) | (id2 << 16) | (id3 << 8) | id4;

   // Parse the TLV we want (AMSS revision)
   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_DMS_RSP, msgID, 1 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1 || pf[0].mValueString.size() <= 0) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   *pFirmwareID = uqcnID;
   eGobiError err = ::MapVersionInfo( uqcnID,
                                      (BYTE)eGOBI_MBN_TYPE_PRI,
                                      pf[0].mValueString.c_str(),
                                      pTechnology,
                                      pCarrier,
                                      pRegion,
                                      pGPSCapability );

   return err;
}

/*===========================================================================
METHOD:
   UpgradeFirmware (Public Method)

DESCRIPTION:
   This function performs the following set of steps:
      a)   Verifies arguments
      b)   Updates firmware ID on device
      c)   Resets the device

   NOTE: Upon successful completion the above steps will have been completed, 
         however the actual upgrade of the firmware will necessarily then 
         follow.

PARAMETERS:
   pDestinationPath  [ I ] - The fully qualified path to the destination folder 
                             that the firmware download service will use 
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::UpgradeFirmware( CHAR * pDestinationPath )
{
   // Validate arguments
   if (pDestinationPath == 0 || pDestinationPath[0] == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Do we have a device ID?
   if (mDeviceNode.empty() == true)
   {
      return eGOBI_ERR_NO_CONNECTION;
   }

   // Use that to validate the image store for this device
   std::string tmpPath( pDestinationPath );
   int tmpPathlen = tmpPath.size();
   if (tmpPath[tmpPathlen - 1] != '/')
   {
      tmpPath += '/';
   }

   std::string imageStore = ::GetImageStore();
   if (tmpPath.find( imageStore ) == std::string::npos)
   {
      return eGOBI_ERR_INVALID_FILE;
   }

   sImageInfo amssInfo;
   sImageInfo uqcnInfo;
   std::vector <sImageInfo> images;
   images = ::GetImagesInfo( tmpPath );

   ULONG imageCount = (ULONG)images.size();
   for (ULONG i = 0; i < imageCount; i++)
   {
      const sImageInfo & ii = images[i];
      if (ii.mImageType == eGOBI_MBN_TYPE_MODEM)
      {
         amssInfo = ii;
      }
      else if (ii.mImageType == eGOBI_MBN_TYPE_PRI)
      {
         uqcnInfo = ii;
      }
   }

   if (uqcnInfo.IsValid() == false)
   {
      return eGOBI_ERR_INVALID_FILE; 
   }

   if (amssInfo.IsValid() == false)
   {
      amssInfo = GetGenericImage( uqcnInfo );
      
      // Still bad?
      if (amssInfo.IsValid() == false)
      {
         return eGOBI_ERR_INVALID_FILE; 
      }
   }

   WORD msgID = (WORD)eQMI_DMS_SET_FIRMWARE_PREF;
   std::vector <sDB2PackingInput> piv;

   std::ostringstream amssIDStr;
   std::ostringstream uqcnIDStr;
   for (ULONG v = 0; v < 16; v++)
   {
      amssIDStr << " " << (ULONG)amssInfo.mImageID[v];
      uqcnIDStr << " " << (ULONG)uqcnInfo.mImageID[v];
   }
   
   // "2 0%s %d \"%s\" 1%s %d \"%s\""
   std::ostringstream tmp;
   tmp << "2 0" << amssIDStr.str() << " " << amssInfo.mVersion.size()
       << " \"" << amssInfo.mVersion << "\" 1" << uqcnIDStr.str()
       << " " << uqcnInfo.mVersion.size() << " \"" 
       << uqcnInfo.mVersion << "\"";
   
   sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, tmp.str().c_str() );
   piv.push_back( pi );


   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );

   // Send the QMI request, check result, and return 
   eGobiError rc = SendAndCheckReturn( eQMI_SVC_DMS, pRequest );
   if (rc != eGOBI_ERR_NONE)
   {
      return rc;
   }

   // Ask device to power down
   rc = SetPower( 5 );
   if (rc != eGOBI_ERR_NONE)
   {
      return eGOBI_ERR_RESET;
   }

   return rc;
}

/*===========================================================================
METHOD:
   GetImageInfo (Public Method)

DESCRIPTION:
   Returns image information obtained from the firmware image located at the
   provided path

PARAMETERS:
   pPath          [ I ] - Location of the firmware image
   pFirmwareID    [ O ] - Firmware ID obtained from the firmware image
   pTechnology    [ O ] - Technology (0xFFFFFFFF if unknown)
   pCarrier       [ O ] - Carrier (0xFFFFFFFF if unknown)
   pRegion        [ O ] - Region (0xFFFFFFFF if unknown)
   pGPSCapability [ O ] - GPS capability (0xFFFFFFFF if unknown)

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetImageInfo( 
   CHAR *                     pPath, 
   ULONG *                    pFirmwareID,
   ULONG *                    pTechnology,
   ULONG *                    pCarrier,
   ULONG *                    pRegion,
   ULONG *                    pGPSCapability )
{
   // Validate arguments
   if ( (pPath == 0)
   ||   (pPath[0] == 0)
   ||   (pFirmwareID == 0)
   ||   (pTechnology == 0) 
   ||   (pCarrier == 0)
   ||   (pRegion == 0)
   ||   (pGPSCapability == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Do we have a device ID?
   if (mDeviceNode.empty() == true)
   {
      return eGOBI_ERR_NO_CONNECTION;
   }

   // Use that to validate the image store for this device
   std::string tmpPath( pPath );
   int tmpPathlen = tmpPath.size();
   if (tmpPath[tmpPathlen - 1] != '/')
   {
      tmpPath += '/';
   }

   std::string imageStore = ::GetImageStore();
   if (tmpPath.find( imageStore ) < 0)
   {
      return eGOBI_ERR_INVALID_FILE;
   }

   std::vector <sImageInfo> images;
   images = ::GetImagesInfo( tmpPath );

   ULONG imageCount = (ULONG)images.size();
   for (ULONG i = 0; i < imageCount; i++)
   {
      const sImageInfo & ii = images[i];
      if (ii.mImageType == eGOBI_MBN_TYPE_PRI)
      {
         *pFirmwareID = ii.mVersionID;
         return ::MapVersionInfo( ii.mVersionID,
                                  (BYTE)ii.mImageType,
                                  ii.mVersion.c_str(),
                                  pTechnology,
                                  pCarrier,
                                  pRegion,
                                  pGPSCapability );
      }
   }

   return eGOBI_ERR_INVALID_FILE;
}

/*===========================================================================
METHOD:
   GetImageStore (Public Method)

DESCRIPTION:
   Returns the image store folder, i.e. the folder co-located with the
   QDL Service executable which (by default) contains one or more carrier 
   specific image subfolders

PARAMETERS:
   pathSize          [ I ] - Maximum number of characters (including NULL
                             terminator) that can be copied to the image
                             store path array
   pImageStorePath   [ O ] - The path to the image store


RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetImageStore(
   WORD                       pathSize,
   CHAR *                     pImageStorePath )
{
   // Do we have a device ID?
   if (mDeviceNode.size() == true)
   {
      return eGOBI_ERR_NO_CONNECTION;
   }

   std::string imageStore = ::GetImageStore();

   // Copy over image store
   LONG strLen = imageStore.size();
   if (pathSize < (ULONG)strLen + 1)
   {
      pImageStorePath[0] = 0;
      return eGOBI_ERR_BUFFER_SZ;
   }

   memcpy( (LPVOID)pImageStorePath, 
           (LPCVOID)imageStore.c_str(), 
           (SIZE_T)strLen );

   pImageStorePath[strLen] = 0;
   return eGOBI_ERR_NONE; 
}
