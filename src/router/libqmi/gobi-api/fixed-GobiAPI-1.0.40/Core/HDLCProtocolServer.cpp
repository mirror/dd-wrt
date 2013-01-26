/*===========================================================================
FILE:
   HDLCProtocolServer.cpp

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
// Include Files
//---------------------------------------------------------------------------
#include "StdAfx.h"
#include "HDLCProtocolServer.h"
#include "HDLC.h"
#include "CRC.h"

#include <vector>

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// cHDLCProtocolServer Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cHDLCProtocolServer (Public Method)

DESCRIPTION:
   Constructor

PARAMETERS:
   rxType      [ I ] - Protocol type to assign to incoming data
   txType      [ I ] - Protocol type to verify for outgoing data
   bufferSzRx  [ I ] - Size of data buffer for incoming data
   logSz       [ I ] - Size of log (number of buffers)

SEQUENCING:
   None (constructs sequencing objects)

RETURN VALUE:
   None
===========================================================================*/
cHDLCProtocolServer::cHDLCProtocolServer(
   eProtocolType              rxType,
   eProtocolType              txType,
   ULONG                      bufferSzRx,
   ULONG                      logSz )
   :  cProtocolServer( rxType, txType, bufferSzRx, logSz ),
      mRxType( rxType ),
      mpEncodedBuffer( 0 ),
      mpRxDecodeBuffer( 0 ),
      mRxDecodeOffset( 0 ),
      mbInEscape( false )
{
   // Allocate decode buffer?
   if (mRxBufferSize > 0)
   {
      mpRxDecodeBuffer = new BYTE[mRxBufferSize * 4];
   }
}

/*===========================================================================
METHOD:
   ~cHDLCProtocolServer (Public Method)

DESCRIPTION:
   Destructor

SEQUENCING:
   None (constructs sequencing objects)

RETURN VALUE:
   None
===========================================================================*/
cHDLCProtocolServer::~cHDLCProtocolServer()
{
   // Free encoded buffer?
   if (mpEncodedBuffer != 0)
   {
      delete mpEncodedBuffer;
      mpEncodedBuffer = 0;
   }

   // Free decode buffer?
   if (mpRxDecodeBuffer != 0)
   {
      delete [] mpRxDecodeBuffer;
      mpRxDecodeBuffer = 0;
   }
}

/*===========================================================================
METHOD:
   InitializeComm (Internal Method)

DESCRIPTION:
   Perform protocol specific communications port initialization

   NOTE: This sends the commands to the driver which sends the IOCTL, but
   that isn't successful

SEQUENCING:
   None (must be called from protocol server thread)

RETURN VALUE:
   bool
===========================================================================*/
bool cHDLCProtocolServer::InitializeComm()
{
   // Default configuration setting   
   struct termios settings;

   if (mComm.GetSettings( &settings ) == false)
   {
      return false;
   }
   
   cfmakeraw( &settings );
   settings.c_cflag |= CREAD|CLOCAL;
   
   return mComm.ConfigureSettings( &settings );
}

/*===========================================================================
METHOD:
   CleanupComm (Internal Method)

DESCRIPTION:
   Perform protocol specific communications port cleanup

SEQUENCING:
   None (must be called from protocol server thread)

RETURN VALUE:
   bool
===========================================================================*/
bool cHDLCProtocolServer::CleanupComm()
{
   // Nothing to actually do here
   return true;
}

/*===========================================================================
METHOD:
   EncodeTxData (Internal Method)

DESCRIPTION:
   Encode data for transmission

PARAMETERS:
   pBuffer        [ I ] - Data to be encoded
   bEncoded       [ O ] - Do we even encoded data?

SEQUENCING:
   None (must be called from protocol server thread)

RETURN VALUE:
   sSharedBuffer * - Encoded data (0 upon error when encoding is indicated)
===========================================================================*/
sSharedBuffer * cHDLCProtocolServer::EncodeTxData( 
   sSharedBuffer *            pBuffer,
   bool &                     bEncoded )
{
   // We encoded data
   bEncoded = true;

   // Last encoded buffer around?
   if (mpEncodedBuffer != 0)
   {
      // Yes free it.  Note that this assumes that the last transmission has
      // concluded since the buffer must exist during transmission.  Since we
      // support one and only one outstanding request this is valid 
      delete mpEncodedBuffer;
      mpEncodedBuffer = 0;
   }

   mpEncodedBuffer = HDLCEncode( pBuffer );
   return mpEncodedBuffer;
}

/*===========================================================================
METHOD:
   DecodeRxData (Internal Method)

DESCRIPTION:
   Decode incoming data into packets returning the last response

PARAMETERS:
   bytesReceived  [ I ] - Number of bytes to decoded
   rspIdx         [ O ] - Log index of last valid response
   bAbortTx       [ O ] - Response aborts current transmission?

SEQUENCING:
   None (must be called from protocol server thread)

RETURN VALUE:
   bool - Was a response received?
===========================================================================*/
bool cHDLCProtocolServer::DecodeRxData( 
   ULONG                      bytesReceived,
   ULONG &                    rspIdx,
   bool &                     bAbortTx )
{
   // Assume failure
   bool bRC = false;
   rspIdx = INVALID_LOG_INDEX;

   // Something to decode from/to?
   if (bytesReceived == 0 || mpRxDecodeBuffer == 0)
   {
      return bRC;
   }

   BYTE val;
   ULONG idx = 0;
   ULONG maxSz = mRxBufferSize * 4;
   
   while (idx < bytesReceived)
   {
      val = mpRxBuffer[idx++];
     
      // Check for target spewing nonsense
      if (mRxDecodeOffset >= maxSz)
      {
         // Reset to beginning
         mRxDecodeOffset = 0;
      }
      
      // Was the previous byte an escape byte?
      if (mbInEscape == true)
      {
         // Yes, handle it
         mbInEscape = false;
            
         val ^= AHDLC_ESC_M;
         mpRxDecodeBuffer[mRxDecodeOffset++] = val;
      }
      else if (val == AHDLC_ESCAPE) 
      {
         // No, but this one is
         mbInEscape = true;
      }
      else if (val == AHDLC_FLAG)
      {
         // Is this a valid frame?
         if ( (mRxDecodeOffset > 0)
         &&   (CheckCRC( mpRxDecodeBuffer, mRxDecodeOffset ) == true) )
         {
            // Yes, extract it (minus CRC) to a shared buffer
            sSharedBuffer * pTmp = 0;
            pTmp = new sSharedBuffer( mpRxDecodeBuffer, 
                                      mRxDecodeOffset - 2, 
                                      (ULONG)mRxType );

            if (pTmp != 0)
            {
               sProtocolBuffer tmpPB( pTmp );
               ULONG tmpIdx = mLog.AddBuffer( tmpPB );

               // Abort?
               bool bTmpAbortTx = IsTxAbortResponse( tmpPB );
               if (bTmpAbortTx == true)
               {
                  bAbortTx = true;
               }
               else
               {
                  // Is this the response we are looking for?
                  bool bRsp = IsResponse( tmpPB );
                  if (bRsp == true)
                  {
                     rspIdx = tmpIdx;
                     bRC = true;
                  }
               }
            }
         }

         // Reset for next frame
         mRxDecodeOffset = 0;
      }  
      else
      {
         // No, just a regular value
         mpRxDecodeBuffer[mRxDecodeOffset++] = val;
      }
   }

   return bRC;
}
