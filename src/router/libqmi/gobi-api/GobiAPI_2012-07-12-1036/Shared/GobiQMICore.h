/*===========================================================================
FILE: 
   GobiQMICore.h

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

/*=========================================================================*/
// Pragmas
/*=========================================================================*/
#pragma once

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "ProtocolBuffer.h"
#include "QMIProtocolServer.h"
#include "SyncQueue.h"
#include "GobiError.h"

/*=========================================================================*/
// Class cGobiQMICore
/*=========================================================================*/
class cGobiQMICore
{
   public:
      // Constructor
      cGobiQMICore();

      // Destructor
      virtual ~cGobiQMICore();

      // Initialize the object
      virtual bool Initialize();

      // Cleanup the object
      virtual bool Cleanup();

      // (Inline) Return the server as determined by the service type
      cQMIProtocolServer * GetServer( eQMIService svc )
      {
         cQMIProtocolServer * pSvr = 0;

         std::map <eQMIService, sServerInfo>::const_iterator pIter;
         pIter = mServers.find( svc );

         if (pIter != mServers.end())
         {
            const sServerInfo & si = pIter->second;
            pSvr = si.mpServer;
         }

         return pSvr;
      };

      // (Inline) Clear last error recorded
      void ClearLastError()
      {
         mLastError = eGOBI_ERR_NONE;
      };

      // (Inline) Get last error recorded
      eGobiError GetLastError()
      {
         return mLastError;
      };

      // (Inline) Return the last recorded error (if this happens to indicate 
      // that no error occurred then return eGOBI_ERR_INTERNAL)
      eGobiError GetCorrectedLastError()
      {
         eGobiError ec = GetLastError();
         if (ec == eGOBI_ERR_NONE)
         {
            ec = eGOBI_ERR_INTERNAL;
         }

         return ec;
      };

      // (Inline) Return the correct QMI error (if this happens to indicate 
      // that no error occurred then return the mapped eQMI_ERR_INTERNAL 
      // value)
      eGobiError GetCorrectedQMIError( ULONG qmiErrorCode )
      {
         ULONG ec = (ULONG)eQMI_ERR_INTERNAL + (ULONG)eGOBI_ERR_QMI_OFFSET;
         if (qmiErrorCode != (ULONG)eQMI_ERR_NONE)
         {
            ec = qmiErrorCode + (ULONG)eGOBI_ERR_QMI_OFFSET;
         }

         return (eGobiError)ec;
      };

      // Connect to the specified Gobi device interface
      virtual std::set <eQMIService> Connect( 
         LPCSTR                     pInterface,
         std::set <eQMIService> &   services );

      // Disconnect from the currently connected device interface
      virtual bool Disconnect();

      // Send a request using the specified QMI protocol server and wait 
      // for (and then return) the response
      eGobiError Send(
         ULONG                      svcID,
         ULONG                      msgID,
         ULONG                      to,
         ULONG                      inLen,
         const BYTE *               pIn,
         ULONG *                    pOutLen,
         BYTE *                     pOut );

      // Cancel the most recent in-progress Send() based operation
      eGobiError CancelSend( 
         ULONG                      svcID,
         ULONG *                    pTXID );

   protected:
      /* Device interface */
      CHAR mInterface[MAX_PATH];

      /* QMI protocol server/protocol server log count */
      struct sServerInfo
      {
         public:
            // Constructor (default)
            sServerInfo()
               :  mpServer( 0 ),
                  mLogsProcessed( 0 ),
                  mRequestID( 0xffffffff ),
                  mRequestTXID( 0xffffffff )
            { };

            // Constructor (parameterized)
            sServerInfo( cQMIProtocolServer * pServer )
               :  mpServer( pServer ),
                  mLogsProcessed( 0 ),
                  mRequestID( 0xffffffff ),
                  mRequestTXID( 0xffffffff )
            { };

            /* Protocol server */
            cQMIProtocolServer * mpServer;

            /* Protocol server logs processed */
            ULONG mLogsProcessed;

            /* Last scheduled request ID */
            ULONG mRequestID;

            /* Last schedule request QMI transaction ID */
            ULONG mRequestTXID;
      };

      /* QMI protocol servers */
      std::map <eQMIService, sServerInfo> mServers;

      /* Last error recorded */
      eGobiError mLastError;
};
