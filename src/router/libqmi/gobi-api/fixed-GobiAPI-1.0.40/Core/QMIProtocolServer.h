/*===========================================================================
FILE:
   QMIProtocolServer.h

DESCRIPTION:
   QMI protocol server
   
PUBLIC CLASSES AND METHODS:
   cQMIProtocolServer

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
===========================================================================*/

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "ProtocolServer.h"
#include "QMIEnum.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// Class cQMIProtocolServer
/*=========================================================================*/
class cQMIProtocolServer : public cProtocolServer
{
   public:
      // Constructor
      cQMIProtocolServer( 
         eQMIService                serviceType,
         ULONG                      bufferSzRx,
         ULONG                      logSz );

      // Destructor
      virtual ~cQMIProtocolServer();

      // Connect to the given QMI service using the configured QMI
      // control file
      bool Connect( LPCSTR pControlFile );

      // (Inline) Return the device MEID
      std::string GetMEID()
      {
         return mMEID;
      };

      // (Inline) Return the QMI service type
      eQMIService GetServiceType()
      {
         return mService;
      };

      // Get device MEID by interfacing to the given device node
      static std::string GetDeviceMEID( std::string deviceNode );

   protected:
      // Validate a request that is about to be scheduled
      virtual bool ValidateRequest( const sProtocolRequest & req );

      // Perform protocol specific communications port initialization
      virtual bool InitializeComm();

      // Perform protocol specific communications port cleanup
      virtual bool CleanupComm();

      // Decode incoming data into packets returning the last response
      virtual bool DecodeRxData( 
         ULONG                      bytesReceived,
         ULONG &                    rspIdx,
         bool &                     bAbortTx );

      // Encode data for transmission
      virtual sSharedBuffer * EncodeTxData( 
         sSharedBuffer *            pBuffer,
         bool &                     bEncoded );

      // Is the passed in data a response to the current request?
      virtual bool IsResponse( const sProtocolBuffer & rsp );

      // (Inline) Is the passed in data a response that aborts the 
      // current request?
      virtual bool IsTxAbortResponse( const sProtocolBuffer & /* rsp */ )
      {
         // QMI doesn't necessarily require this
         return false;
      };

      /* Current transaction ID */
      SHORT mLastTID;

      /* Type of QMI service we are serving */
      eQMIService mService;

      /* Device MEID */
      std::string mMEID;
};
