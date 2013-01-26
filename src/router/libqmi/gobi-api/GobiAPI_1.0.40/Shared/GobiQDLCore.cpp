/*===========================================================================
FILE: 
   GobiQDLCore.cpp

DESCRIPTION:
   QUALCOMM Gobi QDL Based API Core

PUBLIC CLASSES AND FUNCTIONS:
   cGobiQDLCore

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
#include "GobiQDLCore.h"

#include "QDLBuffers.h"
#include "ProtocolNotification.h"
#include "CoreUtilities.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// Default/minimum timeout for QCWWAN QMI requests
const ULONG DEFAULT_GOBI_QDL_TIMEOUT = 4000;
const ULONG MINIMUM_GOBI_QDL_TIMEOUT = 2000;

/*=========================================================================*/
// cGobiQDLCore Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cGobiQDLCore (Public Method)

DESCRIPTION:
   Constructor
  
RETURN VALUE:
   None
===========================================================================*/
cGobiQDLCore::cGobiQDLCore()
   :  mQDL( 512, 512 ),
      mQDLPortNode( "" ),
      mQDLTimeout( DEFAULT_GOBI_QDL_TIMEOUT )
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   ~cGobiQDLCore (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   None
===========================================================================*/
cGobiQDLCore::~cGobiQDLCore()
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   Initialize (Public Method)

DESCRIPTION:
   Initialize the object
  
RETURN VALUE:
   bool
===========================================================================*/
bool cGobiQDLCore::Initialize()
{
   // Nothing to do
   return true;
}

/*===========================================================================
METHOD:
   Cleanup (Public Method)

DESCRIPTION:
   Cleanup the object
  
RETURN VALUE:
   bool
===========================================================================*/
bool cGobiQDLCore::Cleanup()
{
   // Just in case
   CloseQDLPort( false );

   return true;
}

/*===========================================================================
METHOD:
   GetAvailableQDLPorts (Public Method)

DESCRIPTION:
   Return the set of available Gobi QDL ports

RETURN VALUE:
   std::vector <sDeviceID>
===========================================================================*/
std::vector <std::string> cGobiQDLCore::GetAvailableQDLPorts()
{
   std::vector <std::string> devices;

   std::string path = "/sys/bus/usb/devices/";

   std::vector <std::string> files;
   DepthSearch( path,
                2,
                "ttyUSB",
                files );

   int fileNum = files.size();
   for (int i = 0; i < fileNum; i++)
   {
      // Example "/sys/bus/usb/devices/8-1/8-1:1.1/ttyUSB0"
      std::string nodePath = files[i];
      
      int lastSlash = nodePath.find_last_of( "/" );

      // This is what we want to return if everything else matches
      std::string deviceNode = nodePath.substr( lastSlash + 1 );

      // Move down one directory to the interface level
      std::string curPath = nodePath.substr( 0, lastSlash );

      // Read bInterfaceNumber
      int handle = open( (curPath + "/bInterfaceNumber").c_str(), 
                         O_RDONLY );
      if (handle == -1)
      {
         continue;
      }

      char buff[4];
      memset( buff, 0, 4 );
      
      bool bFound = false;
      int ret = read( handle, buff, 2 );
      if (ret == 2)
      {
         // Interface 1 or 0
         ret = strncmp( buff, "01", 2 );
         if (ret == 0)
         {
            bFound = true;
         }
         ret = strncmp( buff, "00", 2 );
         if (ret == 0)
         {
            bFound = true;
         }

      }
      close( handle );

      if (bFound == false)
      {
         continue;
      }

      // Move down one directory to the device level
      curPath = curPath.substr( 0, curPath.find_last_of( "/" ) );

      // Read idVendor
      handle = open( (curPath + "/idVendor").c_str(), O_RDONLY );
      if (handle == -1)
      {
         continue;
      }
      bFound = false;
      ret = read( handle, buff, 4 );
      if (ret == 4)
      {
         ret = strncmp( buff, "05c6", 4 );
         if (ret == 0)
         {
            bFound = true;
         }
      }
      close( handle );

      if (bFound == false)
      {
         continue;
      }

      // Read idProduct
      handle = open( (curPath + "/idProduct").c_str(), O_RDONLY );
      if (handle == -1)
      {
         continue;
      }
      bFound = false;
      ret = read( handle, buff, 4 );
      if (ret == 4)
      {
         ret = strncmp( buff, "920c", 4 );
         if (ret == 0)
         {
            bFound = true;
         }
      }
      close( handle );

      if (bFound == false)
      {
         continue;
      }

      // Success!
      devices.push_back( deviceNode );
   }

   return devices;
}

/*===========================================================================
METHOD:
   SetQDLTimeout (Public Method)

DESCRIPTION:
   Set the timeout for all subsequent QDL transactions

PARAMETERS:
   to          [ I ] - Timeout value (in milliseconds)
   
RETURN VALUE:
   eGobiError
===========================================================================*/
eGobiError cGobiQDLCore::SetQDLTimeout( ULONG to )
{
   if (to < MINIMUM_GOBI_QDL_TIMEOUT)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

    mQDLTimeout = to;
    return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   OpenQDLPort (Public Method)

DESCRIPTION:
   This function opens the specified QDL port of the device

PARAMETERS:
   portID         [ I ] - ID of QDL port to connect to 
   bBARMode       [ I ] - Request boot and recovery mode feature
   pMajorVersion  [ O ] - Major version of the device boot downloader
   pMinorVersion  [ O ] - Minor version of the device boot downloader

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQDLCore::OpenQDLPort( 
   std::string &              portID,
   ULONG                      bBARMode,
   ULONG *                    pMajorVersion, 
   ULONG *                    pMinorVersion )
{
   if (portID.empty() == true || pMajorVersion == 0 || pMinorVersion == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // First disconnect from current port (if any)
   CloseQDLPort( false );

   // Validate port ID
   std::string foundDevice;
   std::vector <std::string> availPorts = GetAvailableQDLPorts();
   for (int index = 0; index < availPorts.size(); index++)
   {
      if (availPorts[index] == portID)
      {
         foundDevice = availPorts[index];
         break;
      }
   }

   if (foundDevice.empty() == true)
   {
      return eGOBI_ERR_INVALID_DEVID;
   }

   // Initialize server (we don't care about the return code
   // since the following Connect() call will fail if we are
   // unable to initialize the server)
   mQDL.Initialize();

   // Connect to the port
   std::string deviceStr = "/dev/" + foundDevice;
   bool bOK = mQDL.Connect( deviceStr.c_str() );
   if (bOK == false)
   {
      return eGOBI_ERR_CONNECT;
   }

   // Store port ID (we are connected)
   mQDLPortNode = foundDevice;

   // Build the hello request
   bool bBARFeature = bBARMode != 0;
   sSharedBuffer * pHelloBuf = sQDLHello::BuildHelloReq( bBARFeature );
   if (pHelloBuf == 0)
   {
      return eGOBI_ERR_MEMORY;
   }

   // Send the hello request and wait for the response
   sProtocolBuffer rsp;
   rsp = SendQDL( pHelloBuf );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Extract major and minor boot downloader versions
   ULONG majVer;
   ULONG minVer;
   sQDLHello helloRsp( rsp.GetSharedBuffer() );
   if (helloRsp.GetBootVersionInfo( majVer, minVer ) == false)
   {
      sQDLError errRsp( rsp.GetSharedBuffer() );
      if (errRsp.IsValid() == true)
      {
         eQDLError qdlErr = errRsp.GetErrorCode();
         return GetCorrectedQDLError( qdlErr );
      }

      return eGOBI_ERR_MALFORMED_RSP;
   }

   // NOTE: in the current firmware implimentation, this cannot happen.
   // No hello response will be received in case of feature mismatch.
   if (bBARFeature == true)
   {
      const sQDLRawHelloRsp * pTmpRsp = helloRsp.GetResponse();
      if (pTmpRsp == 0)
      {
         return eGOBI_ERR_MALFORMED_RSP;
      }

      if ( (pTmpRsp->mFeatures & QDL_FEATURE_BAR_MODE) == 0)
      {
         return eGOBI_ERR_QDL_BAR_MODE;
      }
   }

   *pMajorVersion = majVer;
   *pMinorVersion = minVer;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   CloseQDLPort (Public Method)

DESCRIPTION:
   This function closes the currently open QDL port of the device

PARAMETERS:
   bInformDevice  [ I ] - Inform device that QDL port is being closed? 

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQDLCore::CloseQDLPort( bool bInformDevice )
{
   // Assume success
   eGobiError rc = eGOBI_ERR_NONE;
   if (mQDLPortNode.empty() == true)
   {
      rc = eGOBI_ERR_NO_CONNECTION;
   }
   else if (bInformDevice == true)
   {
      BYTE cmd = (BYTE)eQDL_CMD_SESSION_CLOSE_REQ;
      eProtocolType pt = ePROTOCOL_QDL_TX;

      sSharedBuffer * pReq = 0;
      pReq = new sSharedBuffer( (const BYTE *)&cmd, 1, pt );
      if (pReq == 0)
      {
         rc = eGOBI_ERR_MEMORY;
      }
      else
      {
         sProtocolBuffer rsp = SendQDL( pReq, 0, 0, false );
         rc = GetLastError();
      }
   }

   mQDL.Disconnect();
   mQDL.Exit();

   mQDLPortNode.clear();

   return rc;
}

/*===========================================================================
METHOD:
   GetQDLImagesPreference (Public Method)

DESCRIPTION:
   This function gets the current images preference as reported by the 
   device boot downloader

PARAMETERS:
   pImageListSize [I/O] - Upon input the maximum number of elements that the 
                          image info list can contain.  Upon successful output 
                          the actual number of elements in the image info list
   pImageList     [ O ] - The image info list
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQDLCore::GetQDLImagesPreference( 
   ULONG *                    pImageListSize, 
   BYTE *                     pImageList )
{
   if (pImageListSize == 0 || *pImageListSize == 0 || pImageList == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   BYTE cmd = (BYTE)eQDL_CMD_GET_IMAGE_PREF_REQ;
   eProtocolType pt = ePROTOCOL_QDL_TX;

   sSharedBuffer * pReq = 0;
   pReq = new sSharedBuffer( (const BYTE *)&cmd, 1, pt );
   if (pReq == 0)
   {
      return eGOBI_ERR_MEMORY;
   }

   ULONG maxImages = (ULONG)*pImageListSize;
   *pImageListSize = 0;

   sProtocolBuffer rsp = SendQDL( pReq );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   sQDLGetImagePref prefRsp( rsp.GetSharedBuffer() );
   if (prefRsp.IsValid() == false)
   {
      sQDLError errRsp( rsp.GetSharedBuffer() );
      if (errRsp.IsValid() == true)
      {
         eQDLError qdlErr = errRsp.GetErrorCode();
         return GetCorrectedQDLError( qdlErr );
      }

      return eGOBI_ERR_MALFORMED_RSP;
   }

   std::list <sQDLRawImageID> imageIDs = prefRsp.GetImageIDs();
   ULONG imageCount = (ULONG)imageIDs.size();
   if (imageCount > maxImages)
   {
      imageCount = maxImages;
   }

   sQDLRawImageID * pOutList = (sQDLRawImageID *)pImageList;
   std::list <sQDLRawImageID>::const_iterator pIter = imageIDs.begin();
   for (ULONG i = 0; i < imageCount; i++)
   {
      *pOutList++ = *pIter++;
   }

   *pImageListSize = imageCount;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PrepareQDLImageWrite (Public Method)

DESCRIPTION:
   This function prepares the device boot downloader for an image write

PARAMETERS:
   imageType   [ I ] - Type of image being written 
   imageSize   [ I ] - Size of image being written
   pBlockSize  [I/O] - Upon input the maximum size of image block supported 
                       by host, upon successful output the maximum size of 
                       image block supported by device

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQDLCore::PrepareQDLImageWrite( 
   BYTE                       imageType,
   ULONG                      imageSize,
   ULONG *                    pBlockSize )
{
   eQDLImageType it = (eQDLImageType)imageType;
   if (::IsValid( it ) == false)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   if (pBlockSize == 0 || *pBlockSize == 0 || *pBlockSize > QDL_MAX_CHUNK_SIZE)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   sSharedBuffer * pReq = 0;
   pReq = sQDLOpenUnframed::BuildOpenUnframedReq( it, imageSize, *pBlockSize );
   if (pReq == 0)
   {
      return eGOBI_ERR_MEMORY;
   }

   sProtocolBuffer rsp = SendQDL( pReq );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   ULONG tmp;
   sQDLOpenUnframed openRsp( rsp.GetSharedBuffer() );
   const sQDLRawOpenUnframedRsp * pTmp = openRsp.GetResponse();
   if (pTmp == 0 || openRsp.GetChunkSize( tmp ) == false)
   {
      sQDLError errRsp( rsp.GetSharedBuffer() );
      if (errRsp.IsValid() == true)
      {
         eQDLError qdlErr = errRsp.GetErrorCode();
         return GetCorrectedQDLError( qdlErr );
      }

      return eGOBI_ERR_MALFORMED_RSP;
   }

   if (openRsp.IsSuccess() == false)
   {
      switch ((eQDLOpenStatus)pTmp->mStatus)
      {
         case eQDL_OPEN_STATUS_SIZE:
            return eGOBI_ERR_QDL_OPEN_SIZE;

         case eQDL_OPEN_STATUS_BAD_TYPE:
            return eGOBI_ERR_QDL_OPEN_TYPE;

         case eQDL_OPEN_STATUS_PROTECTION:
            return eGOBI_ERR_QDL_OPEN_PROT;

         case eQDL_OPEN_STATUS_NOT_NEEDED:
            return eGOBI_ERR_QDL_OPEN_SKIP;
      }

      return eGOBI_ERR_QDL_ERR_GENERAL;
   }

   *pBlockSize = tmp;
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   WriteQDLImageBlock (Public Method)

DESCRIPTION:
   This function writes the specified image block to the device

PARAMETERS:
   sequenceNumber [ I ] - Sequence number for image write 
   blockSize      [ I ] - Size of image block
   pImageBlock    [ I ] - Image block
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQDLCore::WriteQDLImageBlock( 
   USHORT                     sequenceNumber,
   ULONG                      blockSize,
   BYTE *                     pImageBlock )
{
   if (blockSize > QDL_MAX_CHUNK_SIZE || pImageBlock == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   sSharedBuffer * pReq = 0;
   pReq = sQDLWriteUnframed::BuildWriteUnframedReq( sequenceNumber, 
                                                    blockSize );

   if (pReq == 0)
   {
      return eGOBI_ERR_MEMORY;
   }

   sProtocolBuffer rsp = SendQDL( pReq, pImageBlock, blockSize );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   sQDLWriteUnframed writeRsp( rsp.GetSharedBuffer() );
   const sQDLRawWriteUnframedRsp * pTmp = writeRsp.GetResponse();
   if (pTmp == 0)
   {
      sQDLError errRsp( rsp.GetSharedBuffer() );
      if (errRsp.IsValid() == true)
      {
         eQDLError qdlErr = errRsp.GetErrorCode();
         return GetCorrectedQDLError( qdlErr );
      }

      return eGOBI_ERR_MALFORMED_RSP;
   }

   if (writeRsp.IsSuccess() == false)
   {
      switch ((eQDLWriteStatus)pTmp->mStatus)
      {
         case eQDL_WRITE_STATUS_CRC:
            return eGOBI_ERR_QDL_CRC;

         case eQDL_WRITE_STATUS_CONTENT:
            return eGOBI_ERR_QDL_PARSING;
      }

      return eGOBI_ERR_QDL_ERR_GENERAL;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   ValidateQDLImages (Public Method)

DESCRIPTION:
   This function requests the device validate the written images

PARAMETERS:
   pImageType  [ O ] - Upon failure this may contain the type of the image
                       that failed validation
  
RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQDLCore::ValidateQDLImages( BYTE * pImageType )
{
   if (pImageType == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   *pImageType = UCHAR_MAX;

   BYTE cmd = (BYTE)eQDL_CMD_SESSION_DONE_REQ;
   eProtocolType pt = ePROTOCOL_QDL_TX;

   sSharedBuffer * pReq = 0;
   pReq = new sSharedBuffer( (const BYTE *)&cmd, 1, pt );
   if (pReq == 0)
   {
      return eGOBI_ERR_MEMORY;
   }

   sProtocolBuffer rsp = SendQDL( pReq );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   sQDLDone doneRsp( rsp.GetSharedBuffer() );
   const sQDLRawDoneRsp * pTmp = doneRsp.GetResponse();
   if (pTmp == 0)
   {
      sQDLError errRsp( rsp.GetSharedBuffer() );
      if (errRsp.IsValid() == true)
      {
         eQDLError qdlErr = errRsp.GetErrorCode();
         return GetCorrectedQDLError( qdlErr );
      }

      return eGOBI_ERR_MALFORMED_RSP;
   }

   if (doneRsp.IsSuccess() == false)
   {  
      *pImageType = pTmp->mImageType;
      switch ((eQDLDoneStatus)pTmp->mStatus)
      {
         case eQDL_DONE_STATUS_AUTH:
            return eGOBI_ERR_QDL_AUTH;

         case eQDL_DONE_STATUS_WRITE:
            return eGOBI_ERR_QDL_WRITE;
      }

      return eGOBI_ERR_QDL_ERR_GENERAL;
   }

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SendQDL (Public Method)

DESCRIPTION:
   Send a QDL request and wait for and return response (if needed)

PARAMETERS:
   pRequest          [ I ] - Request to schedule
   pAuxData          [ I ] - Auxiliary data for request
   auxDataSz         [ I ] - Size of auxiliary data
   bWaitForResponse  [ I ] - Wait for a response?

RETURN VALUE:
   sProtocolBuffer - The response (invalid when no response was received)
===========================================================================*/
sProtocolBuffer cGobiQDLCore::SendQDL(
   sSharedBuffer *            pRequest,
   const BYTE *               pAuxData,
   ULONG                      auxDataSz,
   bool                       bWaitForResponse )
{
   // Clear last error recorded
   ClearLastError();

   // Returned response
   sProtocolBuffer rsp;

   // Validate the arguments
   if (pRequest == 0)
   {
      mLastError = eGOBI_ERR_MEMORY;
      return rsp;
   }

   // We use the event based notification approach
   cSyncQueue <sProtocolNotificationEvent> evts( 12, true );   
   cProtocolQueueNotification pn( &evts );

   // Process up to the indicated timeout
   cEvent & sigEvt = evts.GetSignalEvent();

   // Build the request object
   sProtocolRequest req( pRequest, 0, mQDLTimeout, 1, 1, &pn );
   req.SetAuxiliaryData( pAuxData, auxDataSz );
   if (bWaitForResponse == false)
   {
      req.SetTXOnly();
   }

   // Are we connected?
   if ( (mQDLPortNode.empty() == true)
   ||   (mQDL.IsConnected() == false) )
   {
      mLastError = eGOBI_ERR_NO_CONNECTION;
      return rsp;
   }

   // Grab the log from the server
   const cProtocolLog & protocolLog = mQDL.GetLog();

   // Schedule the request
   ULONG reqID = mQDL.AddRequest( req );
   if (reqID == INVALID_REQUEST_ID)
   {
      mLastError = eGOBI_ERR_REQ_SCHEDULE;
      return rsp;
   }   

   bool bReq = false;
   bool bExit = false;
   DWORD idx;

   // Process up to the indicated timeout
   while (bExit == false)
   {
      int wc = sigEvt.Wait( mQDLTimeout, idx );
      if (wc == ETIME)
      {
         if (bReq == true)
         {
            mLastError = eGOBI_ERR_RESPONSE_TO;
         }
         else
         {
            mLastError = eGOBI_ERR_REQUEST_TO;
         }
         break;
      }
      else if (wc != 0)
      {
         mLastError = eGOBI_ERR_INTERNAL;
         break;
      }

      sProtocolNotificationEvent evt;
      bool bEvt = evts.GetElement( idx, evt );
      if (bEvt == false)
      {
         mLastError = eGOBI_ERR_INTERNAL;
         bExit = true;
         break;
      }

      switch (evt.mEventType)
      {
         case ePROTOCOL_EVT_REQ_ERR:
            mLastError = eGOBI_ERR_REQUEST;
            bExit = true;
            break;

         case ePROTOCOL_EVT_RSP_ERR:
            mLastError = eGOBI_ERR_RESPONSE;
            bExit = true;
            break;
            
         case ePROTOCOL_EVT_REQ_SENT:
         {
            bReq = true;
            if (bWaitForResponse == false)
            {
               // Success!
               bExit = true;
            }
         }
         break;

         case ePROTOCOL_EVT_RSP_RECV:
            // Success!
            rsp = protocolLog.GetBuffer( evt.mParam2 );
            bExit = true;
            break;
      }
   }
   
   if ( (mLastError == eGOBI_ERR_INTERNAL)
   ||   (mLastError == eGOBI_ERR_REQUEST_TO)
   ||   (mLastError == eGOBI_ERR_RESPONSE_TO) )
   {
      // Remove the request as our protocol notification object is
      // about to go out of scope and hence be destroyed
      mQDL.RemoveRequest( reqID );
   }

   return rsp;
}

/*===========================================================================
METHOD:
   GetConnectedPortID (Public Method)

DESCRIPTION:
   Get the device node of the currently connected Gobi device  

PARAMETERS:
   devNode     [ O ] - Device node (IE: ttyUSB0)

RETURN VALUE:
   bool
===========================================================================*/
bool cGobiQDLCore::GetConnectedPortID( std::string & devNode )
{
   // Assume failure
   bool bFound = false;

   devNode.clear();

   // Were we once connected?
   if (mQDLPortNode.size() > 0)
   {
      // Yes, but is our device still present?
      // NOTE: This does not garantee the device did not leave and come back
      std::vector <std::string> devices = GetAvailableQDLPorts();
      ULONG deviceCount = (ULONG)devices.size();

      for (ULONG a = 0; a < deviceCount; a++)
      {
         if (devices[a] == mQDLPortNode)
         {
            devNode = devices[a];

            bFound = true;
            break;
         }
      }

      if (bFound == false)
      {
         mLastError = eGOBI_ERR_NO_DEVICE;
      }
   }
   else
   {
      // We are not connected
      mLastError = eGOBI_ERR_NO_CONNECTION;
   }

   return bFound;
}
