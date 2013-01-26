/*===========================================================================
FILE:
   QDLProtocolServer.cpp

DESCRIPTION:
   QDL protocol packet server
   
PUBLIC CLASSES AND METHODS:
   cQDLProtocolServer

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
#include "QDLProtocolServer.h"
#include "QDLEnum.h"

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

/*=========================================================================*/
// cQDLProtocolServer Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   cQDLProtocolServer (Public Method)

DESCRIPTION:
   Constructor

PARAMETERS:
   bufferSzRx  [ I ] - Size of data buffer for incoming data

SEQUENCING:
   None (constructs sequencing objects)

RETURN VALUE:
   None
===========================================================================*/
cQDLProtocolServer::cQDLProtocolServer( 
   ULONG                      bufferSzRx,
   ULONG                      logSz )
   :  cHDLCProtocolServer( ePROTOCOL_QDL_RX, 
                           ePROTOCOL_QDL_TX,
                           bufferSzRx, 
                           logSz )
{
   // Nothing to do
}

/*===========================================================================
METHOD:
   ~cQDLProtocolServer (Public Method)

DESCRIPTION:
   Destructor

SEQUENCING:
   None (destroys sequencing objects)

RETURN VALUE:
   None
===========================================================================*/
cQDLProtocolServer::~cQDLProtocolServer()
{
   // Nothing to do
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
sSharedBuffer * cQDLProtocolServer::EncodeTxData( 
   sSharedBuffer *            pBuffer,
   bool &                     bEncoded )
{
   // We encoded data
   bEncoded = true;
   if (pBuffer != 0 && pBuffer->IsValid() == true)
   {
      const BYTE * pReqBuf = mpActiveRequest->mRequest.GetBuffer();

      eQDLCommand reqCmd = (eQDLCommand)pReqBuf[0];
      if (reqCmd == eQDL_CMD_WRITE_UNFRAMED_REQ)
      {
         // The write request is not HDLC encoded
         bEncoded = false;
      }
   }

   if (bEncoded == true)
   {
      // Base class can handle HDLC encoding
      return cHDLCProtocolServer::EncodeTxData( pBuffer, bEncoded );
   }

   return 0;
}

/*===========================================================================
METHOD:
   IsResponse (Internal Method)

DESCRIPTION:
   Is the passed in data a response to the current request?

PARAMETERS:
   rsp         [ I ] - Candidate response

SEQUENCING:
   None (must be called from protocol server thread)

RETURN VALUE:
   bool
===========================================================================*/
bool cQDLProtocolServer::IsResponse( const sProtocolBuffer & rsp )
{
   // Assume not
   bool bRC = false;
   if ( (mpActiveRequest == 0) 
   ||   (mpActiveRequest->mRequest.IsValid() == false)
   ||   (mpActiveRequest->mbWaitingForResponse == false)
   ||   (rsp.IsValid() == false) )
   {
      return bRC;
   }

   const BYTE * pReqBuf = mpActiveRequest->mRequest.GetBuffer();
   const BYTE * pRspBuf = rsp.GetBuffer();

   eQDLCommand reqCmd = (eQDLCommand)pReqBuf[0];
   eQDLCommand rspCmd = (eQDLCommand)pRspBuf[0];

   switch (reqCmd)
   {
      case eQDL_CMD_HELLO_REQ:
         if ( (rspCmd == eQDL_CMD_HELLO_RSP)
         ||   (rspCmd == eQDL_CMD_ERROR) )
         {
            bRC = true;
         }
         break; 

      case eQDL_CMD_OPEN_UNFRAMED_REQ:
         if ( (rspCmd == eQDL_CMD_OPEN_UNFRAMED_RSP)
         ||   (rspCmd == eQDL_CMD_ERROR) )
         {
            bRC = true;
         }
         break; 

      case eQDL_CMD_WRITE_UNFRAMED_REQ:
         if ( (rspCmd == eQDL_CMD_WRITE_UNFRAMED_RSP)
         ||   (rspCmd == eQDL_CMD_ERROR) )
         {
            bRC = true;
         }
         break; 

      case eQDL_CMD_SESSION_DONE_REQ:
         if ( (rspCmd == eQDL_CMD_SESSION_DONE_RSP)
         ||   (rspCmd == eQDL_CMD_ERROR) )
         {
            bRC = true;
         }
         break; 

      case eQDL_CMD_DOWNLOAD_REQ:
      case eQDL_CMD_SESSION_CLOSE_REQ:
         if (rspCmd == eQDL_CMD_ERROR)
         {
            bRC = true;
         }
         break;

      case eQDL_CMD_GET_IMAGE_PREF_REQ:
         if ( (rspCmd == eQDL_CMD_GET_IMAGE_PREF_RSP)
         ||   (rspCmd == eQDL_CMD_ERROR) )
         {
            bRC = true;
         }
         break;
   }

   return bRC; 
}

/*===========================================================================
METHOD:
   IsTxAbortResponse (Internal Method)

DESCRIPTION:
   Is the passed in data a response that aborts the current request?

PARAMETERS:
   rsp         [ I ] - Candidate response

SEQUENCING:
   None (must be called from protocol server thread)

RETURN VALUE:
   bool
===========================================================================*/
bool cQDLProtocolServer::IsTxAbortResponse( const sProtocolBuffer & rsp )
{
   // Assume not
   bool bRC = false;
   if ( (mpActiveRequest == 0) 
   ||   (mpActiveRequest->mRequest.IsValid() == false)
   ||   (mpActiveRequest->mbWaitingForResponse == true)
   ||   (rsp.IsValid() == false) )
   {
      return bRC;
   }

   // If we are in the middle of a transmission an we receive an error
   // packet then we abort
   const BYTE * pRspBuf = rsp.GetBuffer();
   eQDLCommand rspCmd = (eQDLCommand)pRspBuf[0];
   if (rspCmd == eQDL_CMD_ERROR)
   {
      bRC = true;
   }

   return bRC; 
}
