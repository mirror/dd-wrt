/*===========================================================================
FILE: 
   GobiQMICoreImg.cpp

DESCRIPTION:
   QUALCOMM Gobi QMI Based API Core (Image Management)

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
#include "GobiQMICore.h"

#include "QMIBuffers.h"

/*=========================================================================*/
// cGobiQMICore Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   GetImagesPreference (Public Method)

DESCRIPTION:
   This function gets the current images preference

PARAMETERS:
   pImageListSize [I/O] - Upon input the size in BYTEs of the image list 
                          array.  Upon success the actual number of BYTEs 
                          copied to the image list array
   pImageList     [ O ] - The image info list array 
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetImagesPreference( 
   ULONG *                    pImageListSize, 
   BYTE *                     pImageList )
{
   // Validate arguments
   if ( (pImageListSize == 0)
   ||   (*pImageListSize == 0)
   ||   (pImageList == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   ULONG maxSz = *pImageListSize;
   *pImageListSize = 0;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_DMS_GET_FIRMWARE_PREF;
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

   // Try to find TLV ID 1
   std::map <ULONG, const sQMIRawContentHeader *> tlvs;
   tlvs = qmiRsp.GetContents();

   std::map <ULONG, const sQMIRawContentHeader *>::const_iterator pIter;
   pIter = tlvs.find( 1 );
   if (pIter == tlvs.end())
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Enough space to copy result?
   const sQMIRawContentHeader * pHdr = pIter->second;
   ULONG needSz = (ULONG)pHdr->mLength;
   if (needSz == 0)
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   *pImageListSize = needSz;
   if (needSz > maxSz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   pHdr++;
   const BYTE * pData = (const BYTE *)pHdr;

   memcpy( (LPVOID)pImageList,
           (LPCVOID)pData,
           (SIZE_T)needSz );

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetImagesPreference (Public Method)

DESCRIPTION:
   This function sets the current images preference

PARAMETERS:
   imageListSize     [ I ] - The size in BYTEs of the image list array
   pImageList        [ I ] - The image list array
   bForceDownload    [ I ] - Force device to download images from host?
   modemIndex        [ I ] - Desired storage index for downloaded modem image
   pImageTypesSize   [I/O] - Upon input the maximum number of elements that 
                             the download image types array can contain.  
                             Upon successful output the actual number of 
                             elements in the download image types array 
   pImageTypes       [ O ] - The download image types array 
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetImagesPreference( 
   ULONG                      imageListSize, 
   BYTE *                     pImageList,
   ULONG                      bForceDownload,
   BYTE                       modemIndex,
   ULONG *                    pImageTypesSize, 
   BYTE *                     pImageTypes )
{
   // Validate arguments
   if ( (imageListSize == 0) 
   ||   (pImageList == 0)
   ||   (pImageTypesSize == 0)
   ||   (*pImageTypesSize == 0)
   ||   (pImageTypes == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   ULONG maxSz = *pImageTypesSize;
   *pImageTypesSize = 0;

   WORD msgID = (WORD)eQMI_DMS_SET_FIRMWARE_PREF;
   std::vector <sDB2PackingInput> piv;

   sProtocolEntityKey pek1( eDB2_ET_QMI_DMS_REQ, msgID, 1 );
   sDB2PackingInput pi1( pek1, pImageList, imageListSize );
   piv.push_back( pi1 );

   BYTE bOverride = 0;
   if (bForceDownload != 0)
   {
      bOverride = 1;
      sProtocolEntityKey pek2( eDB2_ET_QMI_DMS_REQ, msgID, 16 );
      sDB2PackingInput pi2( pek2, &bOverride, 1 );
      piv.push_back( pi2 );
   }

   if (modemIndex != UCHAR_MAX)
   {
      sProtocolEntityKey pek3( eDB2_ET_QMI_DMS_REQ, msgID, 17 );
      sDB2PackingInput pi3( pek3, &modemIndex, 1 );
      piv.push_back( pi3 );
   }

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }

   sProtocolBuffer rsp = Send( eQMI_SVC_DMS, pRequest );
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

   // Try to find TLV ID 1
   std::map <ULONG, const sQMIRawContentHeader *> tlvs;
   tlvs = qmiRsp.GetContents();

   std::map <ULONG, const sQMIRawContentHeader *>::const_iterator pIter;
   pIter = tlvs.find( 1 );
   if (pIter == tlvs.end())
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Enough space to copy result?
   const sQMIRawContentHeader * pHdr = pIter->second;
   ULONG dataLen = (ULONG)pHdr->mLength;
   if (dataLen == 0)
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   pHdr++;
   const BYTE * pData = (const BYTE *)pHdr;
   BYTE typeCount = *pData++;
   if (typeCount != 0)
   {
      if (dataLen != (ULONG)typeCount + 1)
      {
         return eGOBI_ERR_INVALID_RSP;
      }

      *pImageTypesSize = typeCount;
      if (typeCount > maxSz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      memcpy( (LPVOID)pImageTypes,
              (LPCVOID)pData,
              (SIZE_T)typeCount );
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetBARMode (Public Method)

DESCRIPTION:
   This function returns the boot and recovery image download mode

PARAMETERS:
   pBARMode    [ O ] - Boot and recovery image download mode

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetBARMode( ULONG * pBARMode )
{
   // Validate arguments
   if (pBARMode == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_DMS_GET_IMG_DLOAD_MODE;
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
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 16 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() < 1) 
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Populate the state
   *pBARMode = pf[0].mValue.mU32;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SetBARMode (Public Method)

DESCRIPTION:
   This function requests the device enter boot and recovery image download
   mode after the next reset

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::SetBARMode()
{
   WORD msgID = (WORD)eQMI_DMS_SET_IMG_DLOAD_MODE;
   std::vector <sDB2PackingInput> piv;

   sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, "1" );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }

   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_DMS, pRequest );
}

/*===========================================================================
METHOD:
   GetStoredImages (Public Method)

DESCRIPTION:
   This function gets the list of images stored on the device

PARAMETERS:
   pImageListSize [I/O] - Upon input the size in BYTEs of the image list 
                          array.  Upon success the actual number of BYTEs 
                          copied to the image list array
   pImageList     [ O ] - The image info list array 
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetStoredImages( 
   ULONG *                    pImageListSize, 
   BYTE *                     pImageList )
{
   // Validate arguments
   if ( (pImageListSize == 0)
   ||   (*pImageListSize == 0)
   ||   (pImageList == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   ULONG maxSz = *pImageListSize;

   // Assume failure
   *pImageListSize = 0;

   // Generate and send the QMI request
   WORD msgID = (WORD)eQMI_DMS_LIST_FIRMWARE;
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

   // Try to find TLV ID 1
   std::map <ULONG, const sQMIRawContentHeader *> tlvs;
   tlvs = qmiRsp.GetContents();

   std::map <ULONG, const sQMIRawContentHeader *>::const_iterator pIter;
   pIter = tlvs.find( 1 );
   if (pIter == tlvs.end())
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   // Enough space to copy result?
   const sQMIRawContentHeader * pHdr = pIter->second;
   ULONG needSz = (ULONG)pHdr->mLength;

   *pImageListSize = needSz;
   if (needSz > maxSz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   pHdr++;
   const BYTE * pData = (const BYTE *)pHdr;

   memcpy( (LPVOID)pImageList,
           (LPCVOID)pData,
           (SIZE_T)needSz );

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   GetStoredImageInfo (Public Method)

DESCRIPTION:
   This function returns info about the specified image from the device

PARAMETERS:
   imageInfoSize  [ I ] - The size in BYTEs of the image info array
   pImageInfo     [ I ] - The image info array
   pMajorVersion  [ O ] - Major version of compatible boot downloader
   pMinorVersion  [ O ] - Minor version of compatible boot downloader
   pVersionID     [ O ] - Image version ID
   pInfo          [ O ] - Image info string
   pLockID        [ O ] - Image OEM lock ID

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::GetStoredImageInfo( 
   ULONG                      imageInfoSize, 
   BYTE *                     pImageInfo,
   ULONG *                    pMajorVersion, 
   ULONG *                    pMinorVersion,
   ULONG *                    pVersionID,
   CHAR *                     pInfo,
   ULONG *                    pLockID )
{
   // Validate arguments
   if ( (imageInfoSize == 0)
   ||   (pImageInfo == 0)
   ||   (pMajorVersion == 0)
   ||   (pMinorVersion == 0)
   ||   (pVersionID == 0)
   ||   (pInfo == 0)
   ||   (pLockID == 0) )
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pMajorVersion = ULONG_MAX;
   *pMinorVersion = ULONG_MAX;
   *pVersionID = ULONG_MAX;
   *pLockID = ULONG_MAX;
   pInfo[0] = 0;


   WORD msgID = (WORD)eQMI_DMS_GET_FIRMWARE_INFO;
   std::vector <sDB2PackingInput> piv;

   sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, pImageInfo, imageInfoSize );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }

   // Send the QMI request
   sProtocolBuffer rsp = Send( eQMI_SVC_DMS, pRequest );
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

   // Keep track of the number of TLVs we actually processed
   ULONG tlvCount = 0;

   // Parse the TLV we want (by DB key)
   std::vector <sDB2NavInput> tlvs = DB2ReduceQMIBuffer( qmiRsp );
   sProtocolEntityKey tlvKey( eDB2_ET_QMI_DMS_RSP, msgID, 16 );
   cDataParser::tParsedFields pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 2) 
   {
      tlvCount++;
      *pMajorVersion = (ULONG)pf[0].mValue.mU16;
      *pMinorVersion = (ULONG)pf[1].mValue.mU16;
   }

   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_DMS_RSP, msgID, 17 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 2) 
   {
      tlvCount++;
      *pVersionID = pf[0].mValue.mU32;

      LONG strLen = pf[1].mValueString.size();
      if (strLen > 0 && strLen <= 32)
      {
         memcpy( pInfo, pf[1].mValueString.c_str(), strLen );

         if (strLen < 32)
         {
            pInfo[strLen] = 0;
         }
      }
   }

   tlvKey = sProtocolEntityKey( eDB2_ET_QMI_DMS_RSP, msgID, 18 );
   pf = ParseTLV( db, rsp, tlvs, tlvKey );
   if (pf.size() >= 1) 
   {
      tlvCount++;
      *pLockID = pf[0].mValue.mU32;
   }

   if (tlvCount == 0)
   {
      return eGOBI_ERR_INVALID_RSP;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   DeleteStoredImage (Public Method)

DESCRIPTION:
   This function deletes the specified image from the device

PARAMETERS:
   imageInfoSize  [ I ] - The size in BYTEs of the image info array
   pImageInfo     [ I ] - The image info array

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::DeleteStoredImage( 
   ULONG                      imageInfoSize, 
   BYTE *                     pImageInfo )
{
   // Validate arguments
   if (imageInfoSize == 0 || pImageInfo == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   WORD msgID = (WORD)eQMI_DMS_DELETE_FIRMWARE;
   std::vector <sDB2PackingInput> piv;

   sProtocolEntityKey pek( eDB2_ET_QMI_DMS_REQ, msgID, 1 );
   sDB2PackingInput pi( pek, pImageInfo, imageInfoSize );
   piv.push_back( pi );

   // Pack up the QMI request
   const cCoreDatabase & db = GetDatabase();
   sSharedBuffer * pRequest = DB2PackQMIBuffer( db, piv );
   if (pRequest == 0)
   {
      return eGOBI_ERR_MEMORY;
   }

   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_DMS, pRequest );
}
