/*===========================================================================
FILE: 
   GobiQMICoreCAT.cpp

DESCRIPTION:
   QUALCOMM Gobi QMI Based API Core (CAT Service)

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

//---------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Pragmas (pack structs)
//---------------------------------------------------------------------------
#pragma pack( push, 1 )

/*=========================================================================*/
// Struct sCATTerminalResponseHdr
//    Struct to represent a CAT terminal response header
/*=========================================================================*/
struct sCATTerminalResponseHdr
{
   public:
      ULONG mReferenceID;
      USHORT mLength;

      // Terminal response data of 'mLength' follows
};

/*=========================================================================*/
// Struct sCATEnvelopeCommandHdr
//    Struct to represent a CAT envelope command header
/*=========================================================================*/
struct sCATEnvelopeCommandHdr
{
   public:
      USHORT mCommandID;
      USHORT mLength;

      // Envelope command data of 'mLength' follows
};

//---------------------------------------------------------------------------
// Pragmas
//---------------------------------------------------------------------------
#pragma pack( pop )

/*=========================================================================*/
// cGobiQMICore Methods
/*=========================================================================*/

/*===========================================================================
METHOD:
   CATSendTerminalResponse (Public Method)

DESCRIPTION:
   This function sends the terminal response to the device

PARAMETERS:
   refID       [ I ] - UIM reference ID (from CAT event)
   dataLen     [ I ] - Terminal response data length
   pData       [ I ] - Terminal response data

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::CATSendTerminalResponse( 
   ULONG                      refID,
   ULONG                      dataLen,
   BYTE *                     pData )
{
   const ULONG szTransHdr = (ULONG)sizeof( sQMIServiceRawTransactionHeader );
   const ULONG szMsgHdr   = (ULONG)sizeof( sQMIRawMessageHeader );
   const ULONG szTLVHdr   = (ULONG)sizeof( sQMIRawContentHeader ); 
   const ULONG szCATHdr   = (ULONG)sizeof( sCATTerminalResponseHdr );
   
   // Validate arguments
   if (pData == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   BYTE buf[QMI_MAX_BUFFER_SIZE];

   ULONG totalLen = szTransHdr + szMsgHdr+ szTLVHdr + szCATHdr + dataLen;
   if (QMI_MAX_BUFFER_SIZE < totalLen)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pTLV = (sQMIRawContentHeader *)&buf[0];
   pTLV->mTypeID = 1;
   pTLV->mLength = (WORD)(szCATHdr + dataLen);

   sCATTerminalResponseHdr * pCAT = (sCATTerminalResponseHdr *)&buf[szTLVHdr];
   pCAT->mReferenceID = refID;
   pCAT->mLength = (USHORT)dataLen;

   pCAT++;
   if (dataLen > 0)
   {
      memcpy( (LPVOID)pCAT, (LPCVOID)pData, (SIZE_T)dataLen );
   }

   sSharedBuffer * pReq = 0;
   pReq  = sQMIServiceBuffer::BuildBuffer( eQMI_SVC_CAT,
                                           (WORD)eQMI_CAT_SEND_TERMINAL,
                                           false,
                                           false,
                                           &buf[0],
                                           szTLVHdr + szCATHdr + dataLen );

   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_CAT, pReq );
}

/*===========================================================================
METHOD:
   CATSendEnvelopeCommand (Public Method)

DESCRIPTION:
   This function sends the envelope command to the device

PARAMETERS:
   cmdID       [ I ] - Envelope command ID
   dataLen     [ I ] - Envelope command data length
   pData       [ I ] - Envelope command data

RETURN VALUE:
   eGobiError - Return code
===========================================================================*/
eGobiError cGobiQMICore::CATSendEnvelopeCommand( 
   ULONG                      cmdID,
   ULONG                      dataLen,
   BYTE *                     pData )
{
   const ULONG szTransHdr = (ULONG)sizeof( sQMIServiceRawTransactionHeader );
   const ULONG szMsgHdr   = (ULONG)sizeof( sQMIRawMessageHeader );
   const ULONG szTLVHdr   = (ULONG)sizeof( sQMIRawContentHeader ); 
   const ULONG szCATHdr   = (ULONG)sizeof( sCATEnvelopeCommandHdr );
   
   // Validate arguments
   if (pData == 0)
   {
      return eGOBI_ERR_INVALID_ARG;
   }

   BYTE buf[QMI_MAX_BUFFER_SIZE];

   ULONG totalLen = szTransHdr + szMsgHdr+ szTLVHdr + szCATHdr + dataLen;
   if (QMI_MAX_BUFFER_SIZE < totalLen)
   {
      return eGOBI_ERR_BUFFER_SZ;
   }

   sQMIRawContentHeader * pTLV = (sQMIRawContentHeader *)&buf[0];
   pTLV->mTypeID = 1;
   pTLV->mLength = (WORD)(szCATHdr + dataLen);

   sCATEnvelopeCommandHdr * pCAT = (sCATEnvelopeCommandHdr *)&buf[szTLVHdr];
   pCAT->mCommandID = (USHORT)cmdID;
   pCAT->mLength = (USHORT)dataLen;

   pCAT++;
   if (dataLen > 0)
   {
      memcpy( (LPVOID)pCAT, (LPCVOID)pData, (SIZE_T)dataLen );
   }

   sSharedBuffer * pReq = 0;
   pReq  = sQMIServiceBuffer::BuildBuffer( eQMI_SVC_CAT,
                                           (WORD)eQMI_CAT_SEND_ENVELOPE,
                                           false,
                                           false,
                                           &buf[0],
                                           szTLVHdr + szCATHdr + dataLen );

   // Send the QMI request, check result, and return 
   return SendAndCheckReturn( eQMI_SVC_CAT, pReq );
}
