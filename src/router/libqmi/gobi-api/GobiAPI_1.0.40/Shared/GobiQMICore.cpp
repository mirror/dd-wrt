/*===========================================================================
FILE: 
   GobiQMICore.cpp

DESCRIPTION:
   QUALCOMM Gobi QMI Based API Core

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
#include "ProtocolNotification.h"
#include "CoreUtilities.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

// Default timeout for Gobi QMI requests
const ULONG DEFAULT_GOBI_QMI_TIMEOUT = 2000;

/*=========================================================================*/
// Free Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   FindTLV (Free Method)

DESCRIPTION:
   Find the given TLV

PARAMETERS:
   tlvs        [ I ] - TLV parsing input vector
   tlvKey      [ I ] - Key of the TLV that is to be found

RETURN VALUE:
   cDataParser::tParsedFields
===========================================================================*/
sDB2NavInput FindTLV( 
   const std::vector <sDB2NavInput> &  tlvs, 
   const sProtocolEntityKey &          tlvKey )
{
   sDB2NavInput retNI;

   // We need some TLVs to parse and a valid QMI DB key
   ULONG tlvCount = (ULONG)tlvs.size();
   if (tlvCount == 0 || tlvKey.mKey.size() < 3)
   {
      return retNI;
   }
   
   for (ULONG t = 0; t < tlvCount; t++)
   {
      const sDB2NavInput & ni = tlvs[t];
      if (tlvKey.mKey == ni.mKey)
      {
         retNI = ni;
         break;
      }
   }

   return retNI;
}

/*===========================================================================
METHOD:
   ParseTLV (Free Method)

DESCRIPTION:
   Parse the given TLV to fields

PARAMETERS:
   db             [ I ] - Database to use
   qmiBuf         [ I ] - Original buffer containing TLV (locks data)
   tlvs           [ I ] - TLV parsing input vector
   tlvKey         [ I ] - Key of the TLV that is to be parsed
   bFieldStrings  [ I ] - Generate field value strings?

RETURN VALUE:
   cDataParser::tParsedFields
===========================================================================*/
cDataParser::tParsedFields ParseTLV( 
   const cCoreDatabase &               db,
   const sProtocolBuffer &             qmiBuf,
   const std::vector <sDB2NavInput> &  tlvs, 
   const sProtocolEntityKey &          tlvKey,
   bool                                bFieldStrings )
{
   cDataParser::tParsedFields retFields;
   
   // We need some TLVs to parse and a valid QMI DB key
   ULONG tlvCount = (ULONG)tlvs.size();
   if (tlvCount == 0 || tlvKey.mKey.size() < 3)
   {
      return retFields;
   }
   
   for (ULONG t = 0; t < tlvCount; t++)
   {
      const sDB2NavInput & ni = tlvs[t];
      if (tlvKey.mKey == ni.mKey)
      {
         cDataParser dp( db, qmiBuf, tlvKey, ni.mpPayload, ni.mPayloadLen );
         dp.Parse( bFieldStrings, false );

         retFields = dp.GetFields();
         break;
      }
   }

   return retFields;
}

/*=========================================================================*/
// cGobiQMICore Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cGobiQMICore (Public Method)

DESCRIPTION:
   Constructor
  
RETURN VALUE:
   None
===========================================================================*/
cGobiQMICore::cGobiQMICore()
   :  mbFailOnMultipleDevices( false ),
      mDeviceNode( "" ),
      mDeviceKey( "" ),
      mLastError( eGOBI_ERR_NONE ),
      mRequests( 16 ),
      mLastNetStartID( (WORD)INVALID_QMI_TRANSACTION_ID )
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   ~cGobiQMICore (Public Method)

DESCRIPTION:
   Destructor
  
RETURN VALUE:
   BOOL
===========================================================================*/
cGobiQMICore::~cGobiQMICore()
{
   Cleanup();
}

/*===========================================================================
METHOD:
   Initialize (Public Method)

DESCRIPTION:
   Initialize the object
  
RETURN VALUE:
   bool
===========================================================================*/
bool cGobiQMICore::Initialize()
{
   // Initialize database
   mDB.Initialize();
   
   // Allocate configured QMI servers
   bool bOK = true;
   std::set <tServerConfig>::const_iterator pIter = mServerConfig.begin();
   while (pIter != mServerConfig.end())
   {
      cQMIProtocolServer * pSvr = 0;
      pSvr = new cQMIProtocolServer( pIter->first, 8192, 512 );
      if (pSvr == 0)
      {
         if (pIter->second == true)
         {
            bOK = false;
            break;
         }
      }
      else
      {
         mServers[pIter->first] = pSvr;
      }
  
      pIter++;
   }

   if (bOK == false)
   {
      Cleanup(); 
   }

   return bOK;
}

/*===========================================================================
METHOD:
   Cleanup (Public Method)

DESCRIPTION:
   Cleanup the object
  
RETURN VALUE:
   bool
===========================================================================*/
bool cGobiQMICore::Cleanup()
{
   Disconnect();

   // Free allocated QMI servers
   std::map <eQMIService, cQMIProtocolServer *>::const_iterator pIter;
   pIter = mServers.begin();

   while (pIter != mServers.end())
   {
      cQMIProtocolServer * pSvr = pIter->second;
      if (pSvr != 0)
      {
         delete pSvr;
      }

      pIter++;
   }

   mServers.clear();

   return true;
}

/*===========================================================================
METHOD:
   GetAvailableDevices (Public Method)

DESCRIPTION:
   Return the set of available Gobi network devices

RETURN VALUE:
   std::vector <tDeviceID> - Vector of device ID and device key pairs
===========================================================================*/
std::vector <cGobiQMICore::tDeviceID>
cGobiQMICore::GetAvailableDevices()
{
   std::vector <tDeviceID> devices;

   std::string path = "/sys/bus/usb/devices/";

   std::vector <std::string> files;
   DepthSearch( path,
                3,
                "qcqmi",
                files );

   int fileNum = files.size();
   for (int i = 0; i < fileNum; i++)
   {
      // Example "/sys/bus/usb/devices/8-1/8-1:1.0/GobiQMI/qcqmi0"
      std::string nodePath = files[i];
      
      int lastSlash = nodePath.find_last_of( "/" );

      // This is what we want to return if everything else matches
      std::string deviceNode = nodePath.substr( lastSlash + 1 );

      // Move down two directories to the interface level
      std::string curPath = nodePath.substr( 0, lastSlash );
      curPath = curPath.substr( 0, curPath.find_last_of( "/" ) );

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
         ret = strncmp( buff, "00", 2 );
         if (ret == 0)
         {
            bFound = true;
         }

         ret = strncmp( buff, "05", 2 );
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
         ret = strncmp( buff, "920d", 4 );
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

      // Device node success!

      // Get MEID of device node (via ioctl) to use as key
      std::string deviceStr = "/dev/" + deviceNode;
      std::string key = cQMIProtocolServer::GetDeviceMEID( deviceStr );

      tDeviceID device;
      device.first = deviceNode;
      device.second = key;

      devices.push_back( device );
   }

   return devices;
}

/*===========================================================================
METHOD:
   Connect (Public Method)

DESCRIPTION:
   Connect to the specified (or first detected) Gobi device

   Both device node and key are case sensitive

PARAMETERS:
   pDeviceNode   [ I ] - The device node
   pDeviceKey    [ I ] - The device key (unique, stored on-device)
  
RETURN VALUE:
   bool
===========================================================================*/
bool cGobiQMICore::Connect(
   LPCSTR                     pDeviceNode,
   LPCSTR                     pDeviceKey )
{
   // Assume failure
   bool bRC = false;

   // Clear last error recorded
   ClearLastError();

   // If you specify a device key then you have to specify a device ID
   if (pDeviceNode == 0 && pDeviceKey != 0)
   {
      mLastError = eGOBI_ERR_INVALID_ARG;
      return bRC;
   }

   // First we terminate the current connection
   Disconnect();

   // Query system for list of active Gobi devices
   std::vector <tDeviceID> devices = GetAvailableDevices();

   // Did we find any devices?
   ULONG deviceCount = (ULONG)devices.size();
   if (deviceCount == 0)
   {
      mLastError = eGOBI_ERR_NO_DEVICE;
      return bRC;
   }

   std::string deviceKey = "";
   if (pDeviceKey != 0)
   {
      deviceKey = pDeviceKey;
   }

   // Filter that list to include only the specified device?
   if (pDeviceNode != 0)
   {
      std::vector <tDeviceID>::iterator current = devices.begin();
      while (current != devices.end())
      {
         // Remove if device node doesn't match
         if (current->first.compare( pDeviceNode ) != 0)
         {
            // Erase current element and update ourself to point to next
            current = devices.erase( current );
         }
         // Remove if invalid key is specified
         else if (deviceKey.size() != 0
              &&  current->second.compare( deviceKey ) != 0)
         {
            current = devices.erase( current );
         }
         // All necessary parameters match
         else
         {
            current++;
         }
      }
   }

   // Anything left after filtering?
   deviceCount = (ULONG)devices.size();
   if (deviceCount == 0)
   {
      mLastError = eGOBI_ERR_NO_DEVICE;
      return bRC;
   }

   // Too many to choose from?
   if (deviceCount > 1 && mbFailOnMultipleDevices == true)
   {
      mLastError = eGOBI_ERR_MULTIPLE_DEVICES;
      return bRC;
   }

   // Store device ID/key strings
   mDeviceNode = devices[0].first;
   mDeviceKey = devices[0].second;

   // Initalize/connect all configured QMI servers
   std::map <eQMIService, cQMIProtocolServer *>::const_iterator pIter;
   pIter = mServers.begin();

   while (pIter != mServers.end())
   {
      cQMIProtocolServer * pSvr = pIter->second;
      if (pSvr != 0)
      {
         // Initialize server (we don't care about the return code
         // since the following Connect() call will fail if we are
         // unable to initialize the server)
         pSvr->Initialize();

         std::string deviceStr = "/dev/" + mDeviceNode;
         bRC = pSvr->Connect( deviceStr.c_str() );
         if (bRC == false)
         {
            tServerConfig tsc( pIter->first, true );
            if (mServerConfig.find( tsc ) != mServerConfig.end())
            {
               // Failure on essential server
               break;
            }
            else
            {
               // QMI server non-essential (ignore failure)
               bRC = true;
            }
         }
      }

      pIter++;
   }

   // Any server fail?
   if (bRC == false)
   {
      // Yes, disconnect them all
      Disconnect();

      // ... and set the error code
      mLastError = eGOBI_ERR_CONNECT;
   }

   return bRC;
}

/*===========================================================================
METHOD:
   Disconnect (Public Method)

DESCRIPTION:
   Disconnect from the currently connected Gobi device
  
RETURN VALUE:
   bool
===========================================================================*/
bool cGobiQMICore::Disconnect()
{
   // Clear last error recorded
   ClearLastError();
   
   // Assume failure
   bool bRC = false;
   if (mDeviceNode.size() > 0)
   {
      mDeviceNode.clear();
      mDeviceKey.clear();
      bRC = true;
   }
   else 
   {
      mLastError = eGOBI_ERR_NO_CONNECTION;
   }

   // Disconnect/clean-up all configured QMI servers
   std::map <eQMIService, cQMIProtocolServer *>::const_iterator pIter;
   pIter = mServers.begin();

   while (pIter != mServers.end())
   {
      cQMIProtocolServer * pSvr = pIter->second;
      if (pSvr != 0)
      {
         pSvr->Disconnect();
         pSvr->Exit();
      }

      pIter++;
   }

   return bRC;
}

/*===========================================================================
METHOD:
   GetConnectedDeviceID (Public Method)

DESCRIPTION:
   Get the device node/key of the currently connected Gobi device  

PARAMETERS:
   devNode     [ O ] - Device node (IE: qcqmi0)
   devKey      [ O ] - Device key (may be empty)

RETURN VALUE:
   bool
===========================================================================*/
bool cGobiQMICore::GetConnectedDeviceID(
   std::string &                 devNode,
   std::string &                 devKey )
{
   // Clear last error recorded
   ClearLastError();

   // Assume failure
   bool bFound = false;
   devNode.clear();
   devKey.clear();

   // Are all required servers connected?
   bool bAllConnected = true;

   std::map <eQMIService, cQMIProtocolServer *>::const_iterator pIter;
   pIter = mServers.begin();

   while (pIter != mServers.end())
   {
      tServerConfig tsc( pIter->first, true );
      cQMIProtocolServer * pSvr = pIter->second;

      if (mServerConfig.find( tsc ) != mServerConfig.end() && pSvr != 0)
      {
         if (pSvr->IsConnected() == false)
         {
            // Failure on essential server
            bAllConnected = false;
            break;
         }
      }

      pIter++;
   }

   // Were we once connected?
   if (mDeviceNode.size() > 0 && bAllConnected == true)
   {
      // Yes, but is our device still present?
      // NOTE: This does not guarantee the device did not leave and come back
      std::vector <tDeviceID> devices = GetAvailableDevices();
      ULONG deviceCount = (ULONG)devices.size();

      for (ULONG a = 0; a < deviceCount; a++)
      {
         if (devices[a].first == mDeviceNode)
         {
            // If there is a device key specified, it must match.
            if (mDeviceKey.size() > 0)
            {
               if (devices[a].second == mDeviceKey)
               {
                  devNode = devices[a].first;
                  devKey = devices[a].second;

                  bFound = true;
                  break;
               }
            }
            else
            {
               devNode = devices[a].first;

               bFound = true;
               break;
            }
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

/*===========================================================================
METHOD:
   Send (Public Method)

DESCRIPTION:
   Send a request using the specified QMI protocol server and wait for (and
   then return) the response

PARAMETERS:
   svc         [ I ] - QMI service type
   pRequest    [ I ] - Request to schedule
   to          [ I ] - Timeout value (in milliseconds)

RETURN VALUE:
   sProtocolBuffer - The response (invalid when no response was received)
===========================================================================*/
sProtocolBuffer cGobiQMICore::Send(
   eQMIService                svc,
   sSharedBuffer *            pRequest,
   ULONG                      to )
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
   sProtocolRequest req( pRequest, 0, to, 1, 1, &pn );
   if (to == 0)
   {
      mLastError = eGOBI_ERR_INTERNAL;
      return rsp;
   }

   // Grab the server
   cQMIProtocolServer * pSvr = GetServer( svc );
   if (pSvr == 0)
   {
      mLastError = eGOBI_ERR_INTERNAL;
      return rsp;
   }

   // Are we connected?
   if (mDeviceNode.size() <= 0 || pSvr->IsConnected() == false)
   {
      mLastError = eGOBI_ERR_NO_CONNECTION;
      return rsp;
   }

   // Grab the log from the server
   const cProtocolLog & protocolLog = pSvr->GetLog();

   // Schedule the request
   ULONG reqID = pSvr->AddRequest( req );
   if (reqID == INVALID_REQUEST_ID)
   {
      mLastError = eGOBI_ERR_REQ_SCHEDULE;
      return rsp;
   }   

   // Store for external cancel
   tServiceRequest sr( svc, reqID );
   mRequests.AddElement( sr ); 

   bool bReq = false;
   bool bExit = false;
   DWORD idx;

   // Process up to the indicated timeout
   while (bExit == false)
   {
      int wc = sigEvt.Wait( to, idx );
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
            // Are we doing WDS business?
            if (svc == eQMI_SVC_WDS)
            {
               // Grab the as-sent request
               DWORD id = evt.mParam2;
               sProtocolBuffer tmpReq = protocolLog.GetBuffer( id );
               sSharedBuffer * pTmpRequest = tmpReq.GetSharedBuffer();
               if (pTmpRequest != 0)
               {
                  // Check the message ID
                  sQMIServiceBuffer actualReq( pTmpRequest );
                  ULONG msgID = actualReq.GetMessageID();
                  if (msgID == (ULONG)eQMI_WDS_START_NET)
                  {
                     // Grab the transaction ID
                     mLastNetStartID = actualReq.GetTransactionID();
                  }
               }
            }

            bReq = true;
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
      pSvr->RemoveRequest( reqID );
   }

   // Check that the device is still there?
   if ( (mLastError == eGOBI_ERR_REQUEST)
   ||   (mLastError == eGOBI_ERR_RESPONSE)
   ||   (mLastError == eGOBI_ERR_REQUEST_TO)
   ||   (mLastError == eGOBI_ERR_RESPONSE_TO) )
   {
      eGobiError tmp = mLastError;

      std::string dummy;
      GetConnectedDeviceID( dummy, dummy );
      if (mLastError == eGOBI_ERR_NONE)
      {
         mLastError = tmp;
      }
   }

   return rsp;
}

/*===========================================================================
METHOD:
   SendAndCheckReturn (Public Method)

DESCRIPTION:
   Send a request using the specified QMI protocol server and wait for (and
   then validate) the response

PARAMETERS:
   svc         [ I ] - QMI service type
   pRequest    [ I ] - Request to schedule
   to          [ I ] - Timeout value (in milliseconds)

RETURN VALUE:
   eGobiError - Corrected error code
===========================================================================*/
eGobiError cGobiQMICore::SendAndCheckReturn(
   eQMIService                svc,
   sSharedBuffer *            pRequest,
   ULONG                      to )
{
   sProtocolBuffer rsp = Send( svc, pRequest, to );
   if (rsp.IsValid() == false)
   {
      return GetCorrectedLastError();
   }

   // Did we receive a valid QMI response?
   sQMIServiceBuffer qmiRsp( rsp.GetSharedBuffer() );
   if (qmiRsp.IsValid() == false)
   {
      mLastError = eGOBI_ERR_MALFORMED_RSP;
      return mLastError;
   }
            
   // Check the mandatory QMI result TLV for success
   ULONG rc = 0;
   ULONG ec = 0;
   bool bResult = qmiRsp.GetResult( rc, ec );
   if (bResult == false)
   {
      mLastError = eGOBI_ERR_MALFORMED_RSP;
      return mLastError;
   }
   else if (rc != 0)
   {
      return GetCorrectedQMIError( ec );
   }

   // Success!
   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   SendSimple (Public Method)

DESCRIPTION:
   Generate/send a request using the specified QMI protocol server and 
   wait for (and then return) the response

PARAMETERS:
   svc         [ I ] - QMI service type
   msgID       [ I ] - QMI message ID of the request to generate
   to          [ I ] - Timeout value (in milliseconds)

   NOTE: The request has to be a single byte in length, i.e. just a
         command code, in order for success

RETURN VALUE:
   sProtocolBuffer - The response (invalid when no response was received)
===========================================================================*/
sProtocolBuffer cGobiQMICore::SendSimple(
   eQMIService                svc,
   WORD                       msgID,
   ULONG                      to )
{
   // Clear last error recorded
   ClearLastError();

   sProtocolBuffer rsp;
   
   sSharedBuffer * pReq = 0;
   pReq = sQMIServiceBuffer::BuildBuffer( svc, msgID );
   if (pReq == 0)
   {
      mLastError = eGOBI_ERR_MEMORY;
      return rsp;
   }

   rsp = Send( svc, pReq, to );
   return rsp;
}

/*===========================================================================
METHOD:
   CancelSend (Public Method)

DESCRIPTION:
   Cancel the most recent in-progress Send() based operation

RETURN VALUE:
   eGobiError
===========================================================================*/
eGobiError cGobiQMICore::CancelSend()
{
   ULONG reqs = mRequests.GetTotalCount();
   if (reqs == 0)
   {
      return eGOBI_ERR_NO_CANCELABLE_OP;
   }

   tServiceRequest elem( eQMI_SVC_ENUM_BEGIN, INVALID_REQUEST_ID );
   bool bElem = mRequests.GetElement( --reqs, elem );
   if (bElem == false)
   {
      return eGOBI_ERR_INTERNAL;
   }

   cQMIProtocolServer * pSvr = GetServer( elem.first );
   if (pSvr == 0)
   {
      return eGOBI_ERR_INTERNAL;
   }


   bool bRemove = pSvr->RemoveRequest( elem.second );
   if (bRemove == false)
   {
      return eGOBI_ERR_CANCEL_OP;
   }

   return eGOBI_ERR_NONE;
}
