/*===========================================================================
FILE: 
   Gobi3000TranslationCAT.cpp

DESCRIPTION:
   QUALCOMM Translation for Gobi 3000 (Card Application Toolkit Service)

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
#include "Gobi3000Translation.h"

/*===========================================================================
METHOD:
   PackCATSendTerminalResponse

DESCRIPTION:
   This function sends the terminal response to the device

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
   refID       [ I ] - UIM reference ID (from CAT event)
   dataLen     [ I ] - Terminal response data length
   pData       [ I ] - Terminal response data

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackCATSendTerminalResponse( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      refID,
   ULONG                      dataLen,
   BYTE *                     pData )
{
   // Validate arguments
   if (pOut == 0 || pData == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add arguments
   
   // Check size
   WORD tlvx01Sz = sizeof( sCATSendTerminalResponseRequest_TerminalResponseType )
                 + (WORD)dataLen;
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sCATSendTerminalResponseRequest_TerminalResponseType * pTLVx01;
   pTLVx01 = (sCATSendTerminalResponseRequest_TerminalResponseType*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the value
   pTLVx01->mReferenceID = refID;
   pTLVx01->mTerminalResponseLength = (UINT16)dataLen;
   
   offset += sizeof( sCATSendTerminalResponseRequest_TerminalResponseType );

   if (dataLen > 0)
   {
      memcpy( pOut + offset, pData, dataLen );
      offset += dataLen;
   }

   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}

/*===========================================================================
METHOD:
   PackCATSendEnvelopeCommand

DESCRIPTION:
   This function sends the envelope command to the device

PARAMETERS:
   pOutLen     [I/O] - Upon input the maximum number of BYTEs pOut can
                       contain, upon output the number of BYTEs copied
                       to pOut
   pOut        [ O ] - Output buffer
   cmdID       [ I ] - Envelope command ID
   dataLen     [ I ] - Envelope command data length
   pData       [ I ] - Envelope command data

RETURN VALUE:
   ULONG - Return code
===========================================================================*/
ULONG PackCATSendEnvelopeCommand( 
   ULONG *                    pOutLen,
   BYTE *                     pOut,
   ULONG                      cmdID,
   ULONG                      dataLen,
   BYTE *                     pData )
{
   // Validate arguments
   if (pOut == 0 || dataLen == 0 || pData == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   // Add arguments
   
   // Check size
   WORD tlvx01Sz = sizeof( sCATEnvelopeCommandRequest_EnvelopeCommand )
                 + (WORD)dataLen;
   if (*pOutLen < sizeof( sQMIRawContentHeader ) + tlvx01Sz)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pHeader = (sQMIRawContentHeader*)pOut;
   pHeader->mTypeID = 0x01;
   pHeader->mLength = tlvx01Sz;

   ULONG offset = sizeof( sQMIRawContentHeader );

   sCATEnvelopeCommandRequest_EnvelopeCommand * pTLVx01;
   pTLVx01 = (sCATEnvelopeCommandRequest_EnvelopeCommand*)(pOut + offset);
   memset( pTLVx01, 0, tlvx01Sz );

   // Set the value
   pTLVx01->mEnvelopeCommandType = (eQMICATEnvelopeCommandType)cmdID;
   pTLVx01->mEnvelopeLength = (UINT16)dataLen;
   
   offset += sizeof( sCATEnvelopeCommandRequest_EnvelopeCommand );

   if (dataLen > 0)
   {
      memcpy( pOut + offset, pData, dataLen );
      offset += dataLen;
   }

   *pOutLen = offset;

   return eGOBI_ERR_NONE;
}
