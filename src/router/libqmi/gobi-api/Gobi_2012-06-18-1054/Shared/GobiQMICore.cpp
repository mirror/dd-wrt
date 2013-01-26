/*===========================================================================
FILE: 
   GobiQMICore.cpp

DESCRIPTION:
   QUALCOMM Gobi QMI Based API Core

PUBLIC CLASSES AND FUNCTIONS:
   cGobiQMICore

Copyright (c) 2012, Code Aurora Forum. All rights reserved.

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
   :  mLastError( eGOBI_ERR_NONE )
{
   mInterface[0] = 0;
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
bool cGobiQMICore::Cleanup()
{
   Disconnect();
   return true;
}

/*===========================================================================
METHOD:
   Connect (Public Method)

DESCRIPTION:
   Connect to the specified Gobi device
  
PARAMETERS:
   pQMIFile    [ I ] - Gobi device interface to connect to
   services    [ I ] - QMI services to connect to

RETURN VALUE:
   std::set <eQMIService> - Services successfuly configured
===========================================================================*/
std::set <eQMIService> cGobiQMICore::Connect( 
   LPCSTR                     pInterface,
   std::set <eQMIService> &   services )
{
   // The services we successfully connected to
   std::set <eQMIService> retServices;

   // Clear last error recorded
   ClearLastError();

   size_t ifaceLen = strnlen( pInterface, MAX_PATH ) + 1;
   if (ifaceLen >= (size_t)MAX_PATH)
   {
      mLastError = eGOBI_ERR_INVALID_ARG;
      return retServices;
   }

   // Allocate configured QMI servers
   std::set <eQMIService>::const_iterator pIter = services.begin();
   while (pIter != services.end())
   {
      cQMIProtocolServer * pSvr = 0;
      pSvr = new cQMIProtocolServer( *pIter, 8192, 512 );
      if (pSvr != 0)
      {
         // Initialize server (we don't care about the return code
         // since the following Connect() call will fail if we are
         // unable to initialize the server)
         pSvr->Initialize();

         bool bRC = pSvr->Connect( pInterface );
         if (bRC == true)
         {
            sServerInfo si( pSvr );
            std::pair <eQMIService, sServerInfo> entry( *pIter, si ); 
            mServers.insert( entry );

            retServices.insert( *pIter );
         }
      }
   
      pIter++;
   }

   // All servers fail?
   if (retServices.size() == 0)
   {
      // Yes, disconnect them all
      Disconnect();

      // ... and set the error code
      mLastError = eGOBI_ERR_CONNECT;
   }

   memcpy( mInterface, pInterface, ifaceLen );
   return retServices;
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
   
   // Clear device interface
   mInterface[0] = 0;

   // Assume failure
   bool bRC = false;
   if (mServers.size() == 0)
   {
      mLastError = eGOBI_ERR_NO_CONNECTION;
      return bRC;
   }

   // Disconnect/clean-up all configured QMI servers
   std::map <eQMIService, sServerInfo>::iterator pIter;
   pIter = mServers.begin();

   while (pIter != mServers.end())
   {
      sServerInfo & si = pIter->second;
      cQMIProtocolServer * pSvr = si.mpServer;
      if (pSvr != 0)
      {
         pSvr->Disconnect();
         pSvr->Exit();

         delete pSvr;
      }

      si.mLogsProcessed = 0;
      pIter++;
   }

   mServers.clear();

   bRC = true;
   return bRC;
}

/*===========================================================================
METHOD:
   Send (Public Method)

DESCRIPTION:
   Send a request using the specified QMI protocol server and wait for (and
   then return) the response

PARAMETERS:
   svcID       [ I ] - QMI service type
   msgID       [ I ] - QMI message ID
   to          [ I ] - Timeout value (in milliseconds)
   inLen       [ I ] - Length of input buffer
   pIn         [ I ] - Input buffer
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer

RETURN VALUE:
   eGobiError - The result
===========================================================================*/
eGobiError cGobiQMICore::Send(
   ULONG                      svcID,
   ULONG                      msgID,
   ULONG                      to,
   ULONG                      inLen,
   const BYTE *               pIn,
   ULONG *                    pOutLen,
   BYTE *                     pOut )
{
   // Clear last error recorded
   ClearLastError();

   if (msgID > 0xffff)
   {
      mLastError = eGOBI_ERR_INVALID_ARG;
      return mLastError;
   }

   sSharedBuffer * pRequest = 0;
   pRequest = sQMIServiceBuffer::BuildBuffer( (eQMIService)svcID,
                                              (WORD)msgID,
                                              false,
                                              false,
                                              pIn,
                                              inLen );

   if (pRequest == 0)
   {
      mLastError = eGOBI_ERR_MEMORY;
      return mLastError;
   }

   // We use the event based notification approach
   cSyncQueue <sProtocolNotificationEvent> evts( 12, true );   
   cProtocolQueueNotification pn( &evts );

   // Build the request object
   sProtocolRequest req( pRequest, 0, to, 1, 1, &pn );
   if (to == 0)
   {
      mLastError = eGOBI_ERR_INTERNAL;
      return mLastError;
   }

   // Grab the server
   std::map <eQMIService, cGobiQMICore::sServerInfo>::iterator pSvrIter;
   pSvrIter = mServers.find( (eQMIService)svcID );
   if (pSvrIter == mServers.end())
   {
      mLastError = eGOBI_ERR_NO_CONNECTION;
      return mLastError;
   }

   sServerInfo & si = pSvrIter->second;
   cQMIProtocolServer * pSvr = si.mpServer;
   if (pSvr == 0 || pSvr->IsConnected() == false)
   {
      mLastError = eGOBI_ERR_NO_CONNECTION;
      return mLastError;
   }

   // Grab the log from the server
   const cProtocolLog & protocolLog = pSvr->GetLog();

   // Schedule the request
   ULONG reqID = pSvr->AddRequest( req );
   if (reqID == INVALID_REQUEST_ID)
   {
      mLastError = eGOBI_ERR_REQ_SCHEDULE;
      return mLastError;
   }   

   // Store for external cancel
   si.mRequestID = reqID;

   bool bReq = false;
   bool bExit = false;
   DWORD idx;

   // Returned response
   sProtocolBuffer rsp;

   // Process up to the indicated timeout
   cEvent & sigEvt = evts.GetSignalEvent();
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
            // Grab the as-sent request
            DWORD id = evt.mParam2;
            sProtocolBuffer tmpReq = protocolLog.GetBuffer( id );
            sSharedBuffer * pTmpRequest = tmpReq.GetSharedBuffer();
            if (pTmpRequest != 0)
            {
               // Grab the transaction ID
               sQMIServiceBuffer actualReq( pTmpRequest );
               si.mRequestTXID = actualReq.GetTransactionID();
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
   
   // Caller might not be interested in actual output (beyond error code)
   ULONG maxSz = 0;
   if (pOutLen != 0)
   {
      maxSz = *pOutLen;
   }

   if (maxSz > 0)
   {
      // TLV 2 is always present
      ULONG needSz = 0;
      const BYTE * pData = (const BYTE *)qmiRsp.GetRawContents( needSz );
      if (needSz == 0 || pData == 0)
      {
         return eGOBI_ERR_INVALID_RSP;
      }

      *pOutLen = needSz;
      if (needSz > maxSz)
      {
         return eGOBI_ERR_BUFFER_SZ;
      }

      memcpy( pOut, pData, needSz );
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
   CancelSend (Public Method)

DESCRIPTION:
   Cancel the most recent in-progress Send() based operation

PARAMETERS:
   svcID       [ I ] - Service whose outstanding request is to be cancelled
   pTXID       [ O ] - QMI transaction ID of outstanding request

RETURN VALUE:
   eGobiError - The result
===========================================================================*/
eGobiError cGobiQMICore::CancelSend( 
   ULONG                      svcID,
   ULONG *                    pTXID )
{
   // Grab the server
   std::map <eQMIService, cGobiQMICore::sServerInfo>::iterator pSvrIter;
   pSvrIter = mServers.find( (eQMIService)svcID );
   if (pSvrIter == mServers.end())
   {
      mLastError = eGOBI_ERR_NO_CONNECTION;
      return mLastError;
   }

   sServerInfo & si = pSvrIter->second;
   cQMIProtocolServer * pSvr = si.mpServer;
   if (pSvr == 0)
   {
      return eGOBI_ERR_INTERNAL;
   }

   if (si.mRequestID == 0xffffffff)
   {
      return eGOBI_ERR_NO_CANCELABLE_OP;
   }

   bool bRemove = pSvr->RemoveRequest( si.mRequestID );
   if (bRemove == false)
   {
      return eGOBI_ERR_CANCEL_OP;
   }

   if (pTXID != 0)
   {
      *pTXID = si.mRequestTXID;
   }

   return eGOBI_ERR_NONE;
}



