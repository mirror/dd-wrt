/*===========================================================================
FILE:
   HDLCProtocolServer.h

DESCRIPTION:
   Generic HDLC framed protocol packet server
   
PUBLIC CLASSES AND METHODS:
   cHDLCProtocolServer

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

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// Class cHDLCProtocolServer
/*=========================================================================*/
class cHDLCProtocolServer : public cProtocolServer
{
   public:
      // Constructor
      cHDLCProtocolServer( 
         eProtocolType              rxType,
         eProtocolType              txType,
         ULONG                      bufferSzRx,
         ULONG                      logSz );

      // Destructor
      virtual ~cHDLCProtocolServer();

   protected:
      // Perform protocol specific communications port initialization
      virtual bool InitializeComm();

      // Perform protocol specific communications port cleanup
      virtual bool CleanupComm();

      // Encode data for transmission
      virtual sSharedBuffer * EncodeTxData( 
         sSharedBuffer *            pBuffer,
         bool &                     bEncoded );

      // Decode incoming data into packets returning the last response
      virtual bool DecodeRxData( 
         ULONG                      bytesReceived,
         ULONG &                    rspIdx,
         bool &                     bAbortTx );

      // Is the passed in data a response to the current request?
      virtual bool IsResponse( const sProtocolBuffer & /* rsp */ ) = 0;

      // Is the passed in data a response that aborts the current request?
      virtual bool IsTxAbortResponse( const sProtocolBuffer & rsp ) = 0;

      /* Protocol type for incoming data*/
      eProtocolType mRxType;

      /* Encoded data being transmitted */
      sSharedBuffer * mpEncodedBuffer;

      /* Decode buffer for incoming data */
      BYTE * mpRxDecodeBuffer;

      /* Current index into above buffer */
      ULONG mRxDecodeOffset;

      /* Are we currently escaping a character? */
      bool mbInEscape;
};
